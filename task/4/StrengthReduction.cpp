#include "StrengthReduction.hpp"
#include <cmath>

using namespace llvm;

PreservedAnalyses
StrengthReduction::run(Module& mod, ModuleAnalysisManager& mam)
{
    int strengthReductionTimes = 0;

    for (auto& func : mod) {
        for (auto& bb : func) {
            std::vector<Instruction*> instToErase;
            for (auto& inst : bb) {
                if (auto binOp = dyn_cast<BinaryOperator>(&inst)) {
                    Value* lhs = binOp->getOperand(0);
                    Value* rhs = binOp->getOperand(1);
                    auto constLhs = dyn_cast<ConstantInt>(lhs);
                    auto constRhs = dyn_cast<ConstantInt>(rhs);

                    unsigned bitWidth = binOp->getType()->getIntegerBitWidth();

                    // x * 2^n 或 2^n * x => x << n
                    if (binOp->getOpcode() == Instruction::Mul) {
                        int64_t val = 0;
                        Value* other = nullptr;
                        if (constRhs && constRhs->getSExtValue() > 0 && (constRhs->getSExtValue() & (constRhs->getSExtValue() - 1)) == 0) {
                            val = constRhs->getSExtValue();
                            other = lhs;
                        } else if (constLhs && constLhs->getSExtValue() > 0 && (constLhs->getSExtValue() & (constLhs->getSExtValue() - 1)) == 0) {
                            val = constLhs->getSExtValue();
                            other = rhs;
                        }
                        if (val > 0 && other) {
                            unsigned shiftAmount = static_cast<unsigned>(std::log2(val));
                            if (shiftAmount <= 8 && shiftAmount < bitWidth) {
                                Instruction* newInst = BinaryOperator::CreateShl(other, ConstantInt::get(binOp->getType(), shiftAmount));
                                newInst->insertAfter(binOp);
                                binOp->replaceAllUsesWith(newInst);
                                instToErase.push_back(binOp);
                                ++strengthReductionTimes;
                            }
                        }
                    }
                    // x / 2^n => x >> n
                    else if (binOp->getOpcode() == Instruction::SDiv) {
                        if (constRhs && constRhs->getSExtValue() > 0 && (constRhs->getSExtValue() & (constRhs->getSExtValue() - 1)) == 0) {
                            int64_t val = constRhs->getSExtValue();
                            unsigned shiftAmount = static_cast<unsigned>(std::log2(val));
                            if (shiftAmount <= 16 && shiftAmount < bitWidth) {
                                Instruction* newInst = BinaryOperator::CreateAShr(lhs, ConstantInt::get(binOp->getType(), shiftAmount));
                                newInst->insertAfter(binOp);
                                binOp->replaceAllUsesWith(newInst);
                                instToErase.push_back(binOp);
                                ++strengthReductionTimes;
                            }
                        }
                    }
                }
            }
            for (auto* i : instToErase)
                i->eraseFromParent();
        }
    }

    mOut << "StrengthReduction running...\nTo replace " << strengthReductionTimes << " instructions\n";
    return PreservedAnalyses::all();
}