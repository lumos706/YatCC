#include "CommonSubexpressionElimination.hpp"
#include <map>
#include <tuple>

using namespace llvm;

std::vector<Value*> addList;

namespace {
    // 对二元表达式生成唯一key（加法和乘法支持交换律，减法和除法不支持）
    std::tuple<unsigned, Value*, Value*> getBinOpKey(BinaryOperator* binOp) {
        Value* op0 = binOp->getOperand(0);
        Value* op1 = binOp->getOperand(1);
        unsigned opcode = binOp->getOpcode();
        // 只对加法和乘法支持交换律
        if (opcode == Instruction::Add || opcode == Instruction::Mul ||
            opcode == Instruction::And || opcode == Instruction::Or ||
            opcode == Instruction::Xor) {
            if (op0 > op1) std::swap(op0, op1);
        }
        return std::make_tuple(opcode, op0, op1);
    }
}

PreservedAnalyses
CommonSubexpressionElimination::run(Module& mod, ModuleAnalysisManager& mam)
{
    int CommonSubexpressionEliminationTimes = 0;

    // 基于GVN的CSE：支持加法、减法、乘法、除法
    for (auto& func : mod) {
        for (auto& bb : func) {
            std::map<std::tuple<unsigned, Value*, Value*>, Instruction*> valueNumbering;
            std::vector<Instruction*> instToErase;
            for (auto& inst : bb) {
                if (auto* binOp = dyn_cast<BinaryOperator>(&inst)) {
                    unsigned opcode = binOp->getOpcode();
                    if (opcode == Instruction::Add || opcode == Instruction::Sub ||
                        opcode == Instruction::Mul || opcode == Instruction::SDiv ||
                        opcode == Instruction::UDiv || opcode == Instruction::SRem ||
                        opcode == Instruction::URem || opcode == Instruction::And ||
                        opcode == Instruction::Or  || opcode == Instruction::Xor ||
                        opcode == Instruction::Shl || opcode == Instruction::AShr ||
                        opcode == Instruction::LShr) {
                        auto key = getBinOpKey(binOp);
                        auto it = valueNumbering.find(key);
                        if (it != valueNumbering.end()) {
                            binOp->replaceAllUsesWith(it->second);
                            instToErase.push_back(binOp);
                            ++CommonSubexpressionEliminationTimes;
                        } else {
                            valueNumbering[key] = binOp;
                        }
                    }
                }
            }
            for (auto* i : instToErase)
                i->eraseFromParent();
        }
    }

    mOut << "CommonSubexpressionElimination running...\nTo eliminate " << CommonSubexpressionEliminationTimes << " instructions\n";
    return PreservedAnalyses::all();
}