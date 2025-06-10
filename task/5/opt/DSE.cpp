#include "DSE.h"
#include "LICM.h"

// @FIXME: bug when compiling 09_BFS and almost useless
void deadStorageElimination(FunctionPtr func) {
    int removeCount = 0;
    for (auto bbIter = func->basicBlocks.rbegin(); bbIter != func->basicBlocks.rend(); bbIter ++) {
        auto &bb = *bbIter;
        std::set<InstructionPtr> instrToErase;
        for (int i = bb->instructions.size() - 1; i >= 0; i --) {
            auto &ins = bb->instructions[i];
            if (ins->type == Alloca) {
                auto alloc = dynamic_cast<AllocaInstruction *>(ins.get());
                // unused allocation
                if (alloc->des->useHead == nullptr)
                    instrToErase.insert(ins);
            }
            else if (ins->type == Store) {
                auto store = dynamic_cast<StoreInstruction *>(ins.get());
                // cannot predict whether an GEP will be accessed, skip GEP
                if (store->des->getNumUses() > 1 || store->gep != nullptr)
                    continue;
                if (store->des->I != nullptr && store->des->I->type == GEP)
                    continue;
                
                // remove store instruction
                instrToErase.insert(ins);
                Use *toDeleteValueUse = findUse(store->value.get(), ins.get());
                Use *toDeleteDesUse = findUse(store->des.get(), ins.get());
                assert(toDeleteValueUse != nullptr && toDeleteValueUse != nullptr);
                toDeleteValueUse->rmUse();
                toDeleteDesUse->rmUse();

                std::deque<ValuePtr> queue;
                InstructionPtr valInstr = shared_ptr<Instruction>(store->value->I);
                // remove the calculation of store value, if the value is used only once
                if (valInstr != nullptr && store->value->getNumUses() == 1) {
                    if (valInstr->type == Call) {
                        auto call = dynamic_cast<CallInstruction *>(valInstr.get());
                        if(isCallIdempotent(call)) queue.push_back(store->value);
                    }
                    else 
                        queue.push_back(store->value);
                }

                // execute BFS to traverse dead instruction chains
                while (!queue.empty()) {
                    auto val = queue.front();
                    queue.pop_front();
                    Use *use = val->useHead, *nextUse = nullptr;
                    while(use) {
                        nextUse = use->next;
                        ValuePtr user = ValuePtr(use->userVal);
                        assert(user != nullptr);
                        if (user->getNumUses() > 1) {
                            use = nextUse;
                            continue;
                        }
                        queue.push_back(user);
                        instrToErase.insert(InstructionPtr(val->I));
                        use->rmUse();
                        use = nextUse;
                    }
                }
            }
        }
        for (auto &instr: instrToErase) {
            instr->deleteSelfInBB();
            removeCount ++;
        }
    }
}