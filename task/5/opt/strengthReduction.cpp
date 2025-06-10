#include "strengthReduction.h"

bool isExp(int n) {
    if (n & 1) return false;
    int bitCount = 0;
    for(int i = 0; i < 31; i ++) 
    {
        n >>= 1;
        bitCount += (n & 1);
    }
    return (bitCount == 1);
}

void strengthReduction(FunctionPtr func) {
    int regCount = 0;
    for (auto &bb : func->basicBlocks)
    {
        for (int i = 0; i < bb->instructions.size(); i ++)
        {
            auto &ins = bb->instructions[i];
            if (ins->type == InsID::Binary)
            {
                auto binary = dynamic_cast<BinaryInstruction *>(ins.get());
                auto lhs = binary->a;
                auto rhs = binary->b;
                if (binary->op == '*') 
                {
                    if (lhs->isConst && rhs->type->ID == IntID) 
                    {
                        auto const_lhs = dynamic_cast<Const *>(lhs.get());
                        if (const_lhs->intVal > 0 && isExp(const_lhs->intVal))
                        {
                            auto shamt = Const::getConst(Type::getInt(),int(log2(const_lhs->intVal)), "strengthReduction%" + to_string(regCount++));
                            auto new_binary = new BinaryInstruction(rhs, shamt, ',', ins->basicblock);
                            replaceVarByVar(binary->reg, new_binary->reg);
                            bb->instructions[i] = shared_ptr<Instruction>(new_binary);
                        }
                    }
                    else if (rhs->isConst && lhs->type->ID == IntID) 
                    {
                        auto const_rhs = dynamic_cast<Const *>(rhs.get());
                        if (const_rhs->intVal > 0 && isExp(const_rhs->intVal))
                        {
                            auto shamt = Const::getConst(Type::getInt(),int(log2(const_rhs->intVal)), "strengthReduction%" + to_string(regCount++));
                            auto new_binary = new BinaryInstruction(lhs, shamt, ',', ins->basicblock);
                            replaceVarByVar(binary->reg, new_binary->reg);
                            bb->instructions[i] = shared_ptr<Instruction>(new_binary);
                        }
                    }
                }
            }
        }
    }
}
