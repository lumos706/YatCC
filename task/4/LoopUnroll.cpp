#include "LoopUnroll.hpp"

using namespace llvm;

// 辅助函数：判断是否为简单的常数上界计数循环
static bool matchSimpleCountedLoop(BasicBlock *header, BasicBlock *&latch, PHINode *&indVar,
                                   unsigned &tripCount, Value *&stepValue, Value *&boundValue) {
    // 1. 循环头第一个指令为PHI，且有两个前驱
    if (header->phis().empty()) return false;
    indVar = dyn_cast<PHINode>(&*header->phis().begin());
    if (!indVar || indVar->getNumIncomingValues() != 2) return false;

    // 2. 找到latch块（即回跳到header的块）
    latch = nullptr;
    for (auto *pred : predecessors(header)) {
        if (pred != header) {
            latch = pred;
            break;
        }
    }
    if (!latch) return false;

    // 3. latch块最后一条为条件跳转
    auto *br = dyn_cast<BranchInst>(latch->getTerminator());
    if (!br || !br->isConditional()) return false;

    // 4. 条件为icmp，且比较对象之一为PHI
    auto *icmp = dyn_cast<ICmpInst>(br->getCondition());
    if (!icmp) return false;
    Value *cmpL = icmp->getOperand(0);
    Value *cmpR = icmp->getOperand(1);

    // 5. 比较对象之一为PHI，另一个为常数
    ConstantInt *bound = nullptr;
    if (cmpL == indVar && isa<ConstantInt>(cmpR)) {
        bound = dyn_cast<ConstantInt>(cmpR);
    } else if (cmpR == indVar && isa<ConstantInt>(cmpL)) {
        bound = dyn_cast<ConstantInt>(cmpL);
    } else {
        return false;
    }
    if (!bound) return false;
    boundValue = bound;

    // 6. PHI的两个输入，一个来自preheader（初值），一个来自latch（step）
    BasicBlock *preheader = nullptr;
    for (unsigned i = 0; i < 2; ++i) {
        if (indVar->getIncomingBlock(i) != latch)
            preheader = indVar->getIncomingBlock(i);
    }
    if (!preheader) return false;
    ConstantInt *start = dyn_cast<ConstantInt>(indVar->getIncomingValueForBlock(preheader));
    if (!start) return false;

    // 7. latch块中有对indVar的自增/自减
    Instruction *stepInst = nullptr;
    for (auto &I : *latch) {
        if (auto *bin = dyn_cast<BinaryOperator>(&I)) {
            if (bin->getOperand(0) == indVar || bin->getOperand(1) == indVar) {
                stepInst = bin;
                break;
            }
        }
    }
    if (!stepInst) return false;
    stepValue = stepInst->getOperand(0) == indVar ? stepInst->getOperand(1) : stepInst->getOperand(0);
    ConstantInt *stepC = dyn_cast<ConstantInt>(stepValue);
    if (!stepC) return false;

    // 8. 只支持步长为1
    if (stepC->getSExtValue() != 1) return false;

    // 9. 计算trip count
    int64_t s = start->getSExtValue();
    int64_t e = bound->getSExtValue();
    if (e <= s) return false;
    tripCount = e - s;
    if (tripCount > 256) return false; // 防止爆炸

    return true;
}

PreservedAnalyses LoopUnroll::run(Module &mod, ModuleAnalysisManager &mam) {
    int unrollCount = 0;
    for (auto &F : mod) {
        if (F.isDeclaration()) continue;
        std::vector<BasicBlock*> headersToUnroll;

        // 1. 收集所有可展开的循环头
        for (auto &BB : F) {
            BasicBlock *latch = nullptr;
            PHINode *indVar = nullptr;
            unsigned tripCount = 0;
            Value *stepValue = nullptr, *boundValue = nullptr;
            if (matchSimpleCountedLoop(&BB, latch, indVar, tripCount, stepValue, boundValue)) {
                headersToUnroll.push_back(&BB);
            }
        }

        // 2. 展开每个循环
        for (auto *header : headersToUnroll) {
            BasicBlock *latch = nullptr;
            PHINode *indVar = nullptr;
            unsigned tripCount = 0;
            Value *stepValue = nullptr, *boundValue = nullptr;
            if (!matchSimpleCountedLoop(header, latch, indVar, tripCount, stepValue, boundValue)) continue;

            // 找到preheader
            BasicBlock *preheader = nullptr;
            for (unsigned i = 0; i < 2; ++i) {
                if (indVar->getIncomingBlock(i) != latch)
                    preheader = indVar->getIncomingBlock(i);
            }
            if (!preheader) continue;

            // 复制循环体（不包括PHI和终结指令）
            std::vector<Instruction*> bodyInsts;
            for (auto &I : *header) {
                if (!isa<PHINode>(&I) && !I.isTerminator())
                    bodyInsts.push_back(&I);
            }

            // 在preheader后插入展开体
            Instruction *insertPt = preheader->getTerminator();
            IRBuilder<> builder(insertPt);

            for (unsigned iter = 0; iter < tripCount; ++iter) {
                for (auto *inst : bodyInsts) {
                    Instruction *clone = inst->clone();
                    builder.Insert(clone);
                }
            }

            // 移除原循环跳转
            auto *latchBr = dyn_cast<BranchInst>(latch->getTerminator());
            if (latchBr) latchBr->eraseFromParent();

            unrollCount++;
        }
    }

    mOut << "LoopUnroll running...\nUnrolled " << unrollCount << " loops\n";
    return PreservedAnalyses::all();
}