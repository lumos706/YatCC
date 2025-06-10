#include "loopAnalysis.h"

void discoverAndMapSubloop(LoopPtr loop, FunctionPtr func, stack<BasicBlockPtr>& BackedgeTo) {
    while(!BackedgeTo.empty()) {
        auto pred = BackedgeTo.top();
        BackedgeTo.pop();
        auto subLoop = func->getLoopOfBB(pred);
        if(!subLoop) {
            func->addBBToLoop(pred, loop);
            if(pred == loop->getHeader())
                continue;
            for(auto predOfPred : pred->getPredecessor())
                BackedgeTo.push(predOfPred);
        }
        else {
            while(auto parent = subLoop->getParentLoop())
                subLoop = parent;
            if(subLoop == loop)
                continue;
            subLoop->setParentLoop(loop);
            pred = subLoop->getHeader();
            for(auto predOfPred : pred->getPredecessor())
                if(!func->getLoopOfBB(predOfPred) 
                    || func->getLoopOfBB(predOfPred) != subLoop)
                    BackedgeTo.push(predOfPred);
        }
    }
}

int getLoopDepth(BasicBlockPtr bb) {
    auto func = bb->getParent();
    if(auto loop = func->getLoopOfBB(bb))
        return loop->getLoopDepth();
    return 0;
}

void computeExitingBlocks(FunctionPtr func) {
    for (auto loop : func->getAllLoops())  {
        for (auto bb : loop->getLoopBasicBlocks())  {
            auto lastInstr = bb->instructions.back();
            if(auto br = dynamic_cast<BrInstruction *>(lastInstr.get()); br && br->exp) {
                auto bb1 = func->LabelBBMap[br->label_true];
                auto bb2 = func->LabelBBMap[br->label_false];

                if(!loop->contains(bb1) || !loop->contains(bb2))
                {
                    loop->addExitingBlocks(bb);
                }
            }
        }

    }

}

void computeExitBlocks(FunctionPtr func) {

    for (auto loop : func->getAllLoops()) 
        for (auto bb : loop->getLoopBasicBlocks()) 
            for (auto succBB : bb->getSuccessor()) 
            {
                if (!loop->contains(succBB))
                {
                    loop->addExitBlocks(succBB);
                }
            }           
}

void computePreHeader(FunctionPtr func) {
    for (auto loop : func->getAllLoops()) {
        BasicBlockPtr preHeader = nullptr;
        for (auto pred : loop->getHeader()->getPredecessor()) {
            if (getLoopDepth(pred) != loop->getLoopDepth())  {
                preHeader = pred;
                break;
            }
        }
        assert(preHeader != nullptr && preHeader->getSuccessor().size() == 1);
        loop->setPreheader(preHeader);
    }
}

void computeLatch(FunctionPtr func) {
    for (auto loop : func->getAllLoops()) {
        BasicBlockPtr latch = nullptr;
        for (auto pred : loop->getHeader()->getPredecessor()) {
            if (loop->contains(pred)) {
                latch = pred;
                break;
            }
        }
        loop->setLatchBlock(latch);
    }
}

void computeLoopIndVar(FunctionPtr func) {
    for (auto loop : func->getAllLoops()) {
        auto header = loop->getHeader();
        auto lastInstr = header->instructions.back();
        if(auto br = dynamic_cast<BrInstruction *>(lastInstr.get())) {
            if(br->exp == nullptr)
            {
                continue;
            }
                
            auto icmp = dynamic_cast<IcmpInstruction *>(br->exp->I);
            if(!icmp)
            {
                continue;
            }
            loop->setIndexCondInstr(icmp->getSharedThis());
            loop->setICmpKind(icmp->kind);

            function<PhiInstruction*(Instruction*)> getPhi = [&](Instruction* instr) -> PhiInstruction* {
                if(auto phi = dynamic_cast<PhiInstruction*>(instr))
                    return phi;
                else if(auto binary = dynamic_cast<BinaryInstruction*>(instr)) {
                    auto lhs = binary->getOperand(0);
                    auto rhs = binary->getOperand(1);
                    if(auto defInstr = lhs->I; defInstr 
                        && !loop->isSimpleLoopInvariant(lhs))
                        if(auto phi = getPhi(defInstr))
                            return phi;
                    else if(auto defInstr = rhs->I; defInstr 
                        && !loop->isSimpleLoopInvariant(rhs))
                        if(auto phi = getPhi(defInstr))
                            return phi;
                }
                return nullptr;
            };
            auto icmpLhs = icmp->getOperand(0);
            auto icmpRhs = icmp->getOperand(1);
            if(loop->isSimpleLoopInvariant(icmpLhs)) {
                loop->setIndexEnd(icmpLhs);
                if(auto instr = icmpRhs->I; instr 
                    && !loop->isSimpleLoopInvariant(icmpRhs)) {
                    auto phi = getPhi(instr);
                    if(phi)
                        loop->setIndexPhi(phi);
                }
            }
            else if(loop->isSimpleLoopInvariant(icmpRhs)) {
                loop->setIndexEnd(icmpRhs);
                if(auto instr = icmpLhs->I; instr 
                    && !loop->isSimpleLoopInvariant(icmpLhs)) {
                    auto phi = getPhi(instr);
                    if(phi)
                        loop->setIndexPhi(phi);
                }
            }
        }
    }
}

void computeLoopBB(FunctionPtr func) {
    for(auto bb: func->basicBlocks) {
        bb->loop = func->getLoopOfBB(bb);
        if(bb->loop)
            bb->loopDepth = bb->loop->getLoopDepth();
    }
}


void loopAnalysis(FunctionPtr func) {
    func->clearBBToLoop();
    func->clearLoops();

    vector<BasicBlockPtr> postOrderList;
    set<BasicBlockPtr> visited;
    int loopCnt = 0;
    // DFS for dominator Tree to find loops
    BasicBlock::domTreeDFSPost(func->getEntryBlock(), [&](BasicBlockPtr bb) -> bool {
        if(visited.count(bb))
            return true;
        visited.insert(bb);
        return false;
    }, [&](BasicBlockPtr bb) -> void {
        postOrderList.push_back(bb);
    });

    stack<BasicBlockPtr> BackedgeTo;
    for(auto header : postOrderList) {
        for(auto pred : header->getPredecessor())
            if(pred->getAllDoms().count(header)) {
                BackedgeTo.push(pred);
            }
        
        if(!BackedgeTo.empty()) {
            auto loop = LoopPtr(new Loop(header, loopCnt++));
            discoverAndMapSubloop(loop, func, BackedgeTo);
        }
    }

    // DFS for basic blocks to get subloops
    visited.clear();
    BasicBlock::basicBlockDFSPost(func->getEntryBlock(), [&](BasicBlockPtr bb) -> bool {
        if(visited.count(bb))
            return true;
        visited.insert(bb);
        return false;
    }, [&](BasicBlockPtr bb) -> void {
        auto subLoop = func->getLoopOfBB(bb);
        if (subLoop && bb == subLoop->getHeader())  {
            if (subLoop->getParentLoop())
                subLoop->getParentLoop()->addSubLoop(subLoop);
            else {
                func->addTopLoop(subLoop);
                subLoop->setLoopDepth(1); // topLoop is 1
            }

            // reverse but no header
            reverse(subLoop->getLoopBasicBlocksRef().begin() + 1, subLoop->getLoopBasicBlocksRef().end());
            reverse(subLoop->getSubLoopsRef().begin(), subLoop->getSubLoopsRef().end());
            subLoop = subLoop->getParentLoop();
        }

        for(; subLoop; subLoop = subLoop->getParentLoop()) {
            subLoop->addBasicBlock(bb);
        }
    });

    stack<LoopPtr, vector<LoopPtr>> loopStk(func->getTopLoops());
    while (!loopStk.empty()) {
        auto loop = loopStk.top();
        loopStk.pop();
        func->addLoop(loop);
        for(auto subLoop : loop->getSubLoops()) {
            subLoop->setLoopDepth(loop->getLoopDepth() + 1);
            loopStk.push(subLoop);
        }
    }

    computeExitingBlocks(func);
    computeExitBlocks(func);
    computeLatch(func);
    computePreHeader(func);
    computeLoopIndVar(func);
    computeLoopBB(func);

    for(auto loop: func->getTopLoops())
    {
        ScalarEvolution(loop);
    }

}
