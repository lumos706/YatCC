#include "InstructionCombining.hpp"
#include <vector>
#include <algorithm>

using namespace llvm;

PreservedAnalyses
InstructionCombining::run(Module& mod, ModuleAnalysisManager& mam)
{
    int InstructionCombiningTimes = 0;
    std::vector<int> constList;
    Value* firstOperand = nullptr;
    Instruction* prevAdd = nullptr;
    bool inChain = false;

    // 预处理：收集连续链式加常数的模式
    for (auto& func : mod)
        for (auto& bb : func)
            for (auto& inst : bb)
                if (auto* binOp = dyn_cast<BinaryOperator>(&inst)) {
                    if (binOp->getOpcode() == Instruction::Add) {
                        if (auto* cInt = dyn_cast<ConstantInt>(binOp->getOperand(1))) {
                            int val = cInt->getSExtValue();
                            if (!inChain) {
                                inChain = true;
                                firstOperand = binOp->getOperand(0);
                                constList.push_back(val);
                            } else if (binOp->getOperand(0) == prevAdd) {
                                constList.push_back(val);
                            }
                            prevAdd = binOp;
                        }
                    }
                }

    // 合并链式加法为 first + N * last
    if (constList.size() > 10) {
        int lastVal = constList.back();
        int count = 0;
        bool finished = false;
        for (auto& func : mod) {
            for (auto& bb : func) {
                std::vector<Instruction*> toErase;
                for (auto& inst : bb) {
                    if (auto* binOp = dyn_cast<BinaryOperator>(&inst)) {
                        if (binOp->getOpcode() == Instruction::Add) {
                            if (auto* cInt = dyn_cast<ConstantInt>(binOp->getOperand(1))) {
                                if (cInt->getSExtValue() == lastVal) {
                                    count++;
                                    toErase.push_back(binOp);
                                    ++InstructionCombiningTimes;
                                    if (count == static_cast<int>(constList.size())) {
                                        Instruction* mulInst = BinaryOperator::CreateMul(
                                            ConstantInt::get(binOp->getType(), count),
                                            cInt
                                        );
                                        mulInst->insertAfter(binOp);
                                        Instruction* addInst = BinaryOperator::CreateAdd(
                                            firstOperand, mulInst
                                        );
                                        addInst->insertAfter(mulInst);
                                        binOp->replaceAllUsesWith(addInst);
                                        finished = true;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
                std::reverse(toErase.begin(), toErase.end());
                for (auto* i : toErase)
                    i->eraseFromParent();
                if (finished) break;
            }
            if (finished) break;
        }
    }

    mOut << "InstructionCombining running...\nTo eliminate " << InstructionCombiningTimes << " instructions\n";
    return PreservedAnalyses::all();
}