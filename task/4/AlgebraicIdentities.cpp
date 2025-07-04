#include "AlgebraicIdentities.hpp"

using namespace llvm;

PreservedAnalyses
AlgebraicIdentities::run(Module& mod, ModuleAnalysisManager& mam)
{
    int AlgebraicIdentitiesCount = 0;

    for (auto& function : mod) {
        for (auto& basicBlock : function) {
            std::vector<Instruction*> instructionsToErase;
            for (auto& inst : basicBlock) {
                if (auto* binOp = dyn_cast<BinaryOperator>(&inst)) {
                    Value* left = binOp->getOperand(0);
                    Value* right = binOp->getOperand(1);
                    auto* constLeft = dyn_cast<ConstantInt>(left);
                    auto* constRight = dyn_cast<ConstantInt>(right);

                    auto eraseAndReplace = [&](Value* newVal) {
                        binOp->replaceAllUsesWith(newVal);
                        instructionsToErase.push_back(binOp);
                        ++AlgebraicIdentitiesCount;
                    };

                    switch (binOp->getOpcode()) {
                        case Instruction::Add:
                            // x + 0 = x, 0 + x = x
                            if (constRight && constRight->isZero())
                                eraseAndReplace(left);
                            else if (constLeft && constLeft->isZero())
                                eraseAndReplace(right);
                            break;
                        case Instruction::Sub:
                            // x - 0 = x
                            if (constRight && constRight->isZero())
                                eraseAndReplace(left);
                            break;
                        case Instruction::Mul:
                            // x * 0 = 0, 0 * x = 0, x * 1 = x, 1 * x = x
                            if (constRight) {
                                if (constRight->isZero())
                                    eraseAndReplace(ConstantInt::get(binOp->getType(), 0));
                                else if (constRight->isOne())
                                    eraseAndReplace(left);
                            } else if (constLeft) {
                                if (constLeft->isZero())
                                    eraseAndReplace(ConstantInt::get(binOp->getType(), 0));
                                else if (constLeft->isOne())
                                    eraseAndReplace(right);
                            }
                            break;
                        case Instruction::SDiv:
                            // x / 1 = x, 0 / x = 0
                            if (constRight && constRight->isOne())
                                eraseAndReplace(left);
                            else if (constLeft && constLeft->isZero())
                                eraseAndReplace(ConstantInt::get(binOp->getType(), 0));
                            break;
                        case Instruction::SRem:
                            // x % 1 = 0, 0 % x = 0
                            if (constRight && constRight->isOne())
                                eraseAndReplace(ConstantInt::get(binOp->getType(), 0));
                            else if (constLeft && constLeft->isZero())
                                eraseAndReplace(ConstantInt::get(binOp->getType(), 0));
                            break;
                        default:
                            break;
                    }
                }
            }
            for (auto* inst : instructionsToErase)
                inst->eraseFromParent();
        }
    }

    mOut << "AlgebraicIdentities running...\nTo eliminate " << AlgebraicIdentitiesCount << " instructions\n";
    return PreservedAnalyses::all();
}