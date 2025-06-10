#include "LICM.h"

// #define DEBUG

std::unordered_map<FunctionPtr, bool> idemFunctions;

bool isCallIdempotent(CallInstruction *ins) {
    int numOperands = ins->getNumOperands();
    bool isIdempotent = true;
    // traverse all arguments
    for(int i = 0; i < numOperands; i ++) {
        auto arg = ins->getOperand(i);
        // global variable or pointer
        if(dynamic_cast<Variable *>(arg.get())) {
            auto var = dynamic_cast<Variable *>(arg.get());
            if(var->isGlobal) {
                isIdempotent = false;
                break;
            }
            else if(var->type->getID() == PtrID || (var->I && var->I->type == GEP)) {
                isIdempotent = false;
                break;
            }
        }
        // pointer
        else if(dynamic_cast<Const *>(arg.get())) {
            auto constant = dynamic_cast<Const *>(arg.get());
            if(constant->type->getID() == PtrID) {
                isIdempotent = false;
                break;
            }
        }
        else {
            isIdempotent = true;
            break;
        }
    }
    if(!isIdempotent) 
        return false;
    
    auto call = ins;
    FunctionPtr f = call->func;
    if(idemFunctions.find(f) != idemFunctions.end()) {
        isIdempotent = idemFunctions[f];
    }
    else {
        // to avoid recursive callling, mark current function 'f' as false
        idemFunctions[f] = false;
        if(f->isLib)
            isIdempotent = false;
        else {
            for(auto basicBlock: f->basicBlocks) {
                for(auto instr: basicBlock->instructions) {
                    if(instr->type == Load) {
                        auto load = dynamic_cast<LoadInstruction *>(instr.get());
                        if(dynamic_cast<Variable *>(load->from.get()) || load->gep) {
                            isIdempotent = false;
                            break;
                        }
                    }
                    else if(instr->type == Store) {
                        auto store = dynamic_cast<StoreInstruction *>(instr.get());
                        if(dynamic_cast<Variable *>(store->des.get()) || store->gep) {
                            isIdempotent = false;
                            break;
                        }
                    }
                    else if(instr->type == GEP) {
                        isIdempotent = false;
                        break;
                    }
                    else if(instr->type == Call) {
                        auto innerCall = dynamic_cast<CallInstruction *>(instr.get());
                        if(idemFunctions.find(innerCall->func) != idemFunctions.end()) {
                            if(idemFunctions[innerCall->func] == false) {
                                isIdempotent = false;
                                break;
                            }
                        }
                        else {
                            isIdempotent = isCallIdempotent(innerCall);
                            if(!isIdempotent) break;
                        }
                    }
                }
                if(!isIdempotent) break;
            }
        }
        idemFunctions[f] = isIdempotent;
    }
    return isIdempotent;
}

// determinate whether the execution of an instruction will incur effects besides calculating
bool isSafeToSpeculativelyExecute(InstructionPtr instr) {
    auto type = instr->type;
    bool safe = true;
    switch(type) {
        // if new instruction is added, remember to modify
        default: {
            safe = true;
        } break;

        // @TODO: ignore overflow when INT_MIN * (-1)
        case Binary: {
            auto binary = dynamic_cast<BinaryInstruction *>(instr.get());
            char op = binary->op;
            if(op == '/' || op == '%') {
                auto b = binary->b;
                // denominator should not be 0
                if(dynamic_cast<Const *>(b.get())) {
                    auto constVal = dynamic_cast<Const *>(b.get());
                    if(constVal->type->getID() == IntID)
                        assert(constVal->intVal != 0);
                    else if(constVal->type->getID() == FLoatID)
                        assert(constVal->floatVal != 0);
                }
                safe = true;
            }
        } break;

        // @TODO: always regard pointer as accessable pointer(point to legal memory)
        case Load: {
            safe = true;
        } break;

        // @TODO: use isSpeculative instead isCallIdempotent
        case Call: {
            auto call = dynamic_cast<CallInstruction *>(instr.get());
            safe = isCallIdempotent(call);
        }
            
        case Alloca:
        case Phi:
        case Return:
        case Br:
        case Store: {
            safe = false;
        } break;
    }
    return safe;
}

bool safeToHoist(InstructionPtr instr, LoopPtr curLoop, FunctionPtr func) {
    bool cond1 = isSafeToSpeculativelyExecute(instr);
    // handled in isLoopInvariant
    if(instr->type == Call) 
        cond1 = true;
    unordered_set<BasicBlockPtr> exitBlocks = curLoop->getExitBlocks();
    auto instrBlock = instr->basicblock;
    auto entry = func->basicBlocks[0];
    bool cond2 = std::accumulate(exitBlocks.begin(), exitBlocks.end(), true, 
                                [&](bool acc, BasicBlockPtr exitBlock) {
                                    return acc && Loop::isADominatorB(instrBlock, exitBlock, entry);
                                });
    return cond1 || cond2;
}

bool isArgInCall(ValuePtr value, CallInstruction *call) {
    int numArg = call->argv.size();
    for(int i = 0; i < numArg; i ++) {
        auto arg = call->getOperand(i);
        if(arg == value)
            return true;

        auto argGEP = dynamic_cast<GetElementPtrInstruction *>(arg->I);
        if(argGEP && argGEP->from == value)
            return true;
    }
    return false;
}

// determinate whether the loaded variable '%v' in Inst '%0 = load i32, i32* %v' will be changed in loop blocks
bool isChangedInBlock(LoadInstruction *load, LoopPtr curLoop, FunctionPtr func) {
    vector<BasicBlockPtr> loopBlocks = curLoop->getLoopBasicBlocks();
    auto from = load->from;
    auto fromVar = dynamic_cast<Variable *>(from.get());
    // if load from GEP, avoid storing to same base
    while(dynamic_cast<GetElementPtrInstruction *>(from->I)) {
        from = dynamic_cast<GetElementPtrInstruction *>(from->I)->from;
    }

    for(auto bb: loopBlocks) {
        for(auto loopInstr: bb->instructions) {
            if(loopInstr->type == Store) {
                auto store = dynamic_cast<StoreInstruction *>(loopInstr.get());
                auto des = store->des;
                if(from == des) 
                    return true;

                // store to same array base
                auto desGEP = dynamic_cast<GetElementPtrInstruction *>(des->I);
                while(desGEP) {
                    if(desGEP->from == from)
                        return true;
                    desGEP = dynamic_cast<GetElementPtrInstruction *>(desGEP->from->I);
                }
            }
            else if(loopInstr->type == Call) {
                auto call = dynamic_cast<CallInstruction *>(loopInstr.get());
                // if variable '%v' is passed as function call parameter, return true
                if(isArgInCall(from, call))
                    return true;

                // if 'loadFrom' is a global variable, determinate whether it's changed(stored) in the called function
                FunctionPtr f = call->func;
                // lib in sysy means IO, which doesn't change the value a pointer points to
                if(f->isLib)
                    continue;
                if(fromVar && fromVar->isGlobal) {
                    for(auto calledBB: f->basicBlocks) {
                        for(auto calledInstr: calledBB->instructions) {
                            if(calledInstr->type == Store) {
                                auto store = dynamic_cast<StoreInstruction *>(calledInstr.get());
                                auto des = store->des;
                                if(from == des) 
                                    return true;

                                // call to same array base
                                auto desGEP = dynamic_cast<GetElementPtrInstruction *>(des->I);
                                if(desGEP) {
                                }
                                if(desGEP && desGEP->from == from) {
                                    return true;
                                }
                            }
                            else if(calledInstr->type == Call) {
                                auto call = dynamic_cast<CallInstruction *>(calledInstr.get());
                                return true;
                            }
                        }
                    }
                }
            }
        }
    }
    return false;
}

bool isLoopInvariant(InstructionPtr instr, LoopPtr curLoop, FunctionPtr func, set<Instruction *>& toDelete) {
    auto type = instr->type;
    // condition 1: One of the invariant instruction classes
    bool cond1 = (type == Alloca
        || (type == Load && !isChangedInBlock(dynamic_cast<LoadInstruction *>(instr.get()), curLoop, func))
        || (type == Call && isCallIdempotent(dynamic_cast<CallInstruction *>(instr.get())))
        || type == Bitcast || type == Sitofp || type == Fptosi
        || type == GEP || type == Binary || type == Fneg || type == Ext);
    
    // condition 2: each operand is either a constant or an instruction computed outside the loop
    bool cond2 = true;
    int numOperand = instr->getNumOperands();
    for(int i = 0; i < numOperand; i ++) {
        // @TODO: PhiNode should be handled specially
        if(type == Phi) {
            cond2 = false;
            break;
        }
        auto operand = instr->getOperand(i);
        bool isConst = operand->isConst;
        // 'operand->I == nullptr' means allocated variable
        bool isComputedOutside = (operand->I == nullptr 
            || !curLoop->contains(operand->I->basicblock)
            || toDelete.find(operand->I) != toDelete.end());
        #ifdef DEBUG
        cout << "  " << operand->getStr() << " isConst: " << isConst << ", isComputedOutside: " << isComputedOutside << endl;
        if(operand->I) operand->I->print();
        else printf("operand->I == nullptr");
        if(operand->I && !curLoop->contains(operand->I->basicblock)) {
            printf("not in loop, instruction in basicblock: %s\n", operand->I->basicblock->label->name.c_str());
        }
        if(toDelete.find(operand->I) != toDelete.end()) printf("has been deleted\n");
        #endif
        cond2 = cond2 && (isConst || isComputedOutside);
    }
    #ifdef DEBUG
    printf("isLoopInvariant cond1: %d, cond2: %d\n", cond1, cond2);
    #endif
    return cond1 && cond2;
}

bool canMoveExit(InstructionPtr instr, LoopPtr curLoop, FunctionPtr func, vector<InstructionPtr> &toMoveExitInst) {
    if(instr->type == Binary) {
        auto binary = dynamic_cast<BinaryInstruction *>(instr.get());
        if(binary->op == '%' && binary->b->isConst) {
            
        }
    }
}

void runForLoop(LoopPtr curLoop, int& moveCount, FunctionPtr func) {
    auto entry = func->basicBlocks[0];
    BasicBlockPtr preheader = curLoop->getPreheader();
    queue<BasicBlockPtr> q;
    set<BasicBlockPtr> visited;
    visited.insert(preheader);
    for(auto childPair: preheader->dominatorSon) {
        auto childBB = childPair;
        // check if childBB is outside the current loop or in sub loop
        if(!curLoop->contains(childBB) || childBB->loop != curLoop)
            continue;
        q.push(childBB);
    }
    #ifdef DEBUG
    cout << "preheader of Loop " << curLoop->header->label->name << ": " << preheader->label->name << endl;
    cout << "basic blocks in loop: ";
    for(auto bb: curLoop->getLoopBasicBlocks()) {
        cout << bb->label->name << ", ";
    }
    cout << endl;
    #endif
    while(!q.empty()) {
        BasicBlockPtr bb = q.front();
        q.pop();
        if(visited.find(bb) != visited.end())
            continue;
        visited.insert(bb);
        for(auto childPair: bb->dominatorSon) {
            auto childBB = childPair;
            // check if childBB is outside the current loop or in sub loop
            if(!curLoop->contains(childBB) || childBB->loop != curLoop)
                continue;
            q.push(childBB);
        }

        // hoist
        int instrCnt = bb->instructions.size();
        set<Instruction *> searchDelete;
        vector<InstructionPtr> toDeleteInstr;
        vector<InstructionPtr> toMoveExitInstr;
        for(int i = 0; i < instrCnt; i ++) {
            auto instr = bb->instructions[i];
            #ifdef DEBUG
            cout << endl << "  " << i << "/" << instrCnt << ": ";
            instr->print();
            #endif
            bool cond1 = isLoopInvariant(instr, curLoop, func, searchDelete);
            #ifdef DEBUG
            cout << "cond1: " << cond1 << ", ";
            #endif
            bool cond2 = safeToHoist(instr, curLoop, func);
            #ifdef DEBUG
            cout << "cond2: " << cond2 << endl;
            #endif
            if(cond1 && cond2) {
                toDeleteInstr.push_back(instr);
                searchDelete.insert(instr.get());
                preheader->pushInstruction(instr);
                #ifdef DEBUG
                // cout << "to delete instruction: ";
                // instr->print();
                #endif
            }
        }
        if(toDeleteInstr.size() > 0) {
            preheader->pushInstruction(toDeleteInstr);
            #ifdef DEBUG
            cout << "instructions to be hoist:" << endl;
            for(auto instr: toDeleteInstr) {
                instr->print();
            }
            #endif

            vector<InstructionPtr> newInstructions;
            for(int i = 0; i < bb->instructions.size(); i ++) {
                if(searchDelete.find(bb->instructions[i].get()) == searchDelete.end()) {
                    newInstructions.emplace_back(bb->instructions[i]);
                }
            }
            bb->instructions = newInstructions;
            moveCount += toDeleteInstr.size();
        }
    }
}

void LICM(FunctionPtr func) {
    int moveCount = 0;
    vector<LoopPtr> postorderLoops = func->getLoopsInPostorder();
    int numLoops = postorderLoops.size();
    for(int i = 0; i < numLoops; i ++) {
        runForLoop(postorderLoops[i], moveCount, func);
    }
}
