#include "computeUse.h"

bool hasUser(InsID id) {
    return !(id == Return || id == Br || id == Store || id == Call);
}

void computeUse(FunctionPtr func) {
    // delete all use
    for(auto arg: func->formArguments) {
        arg->useHead = nullptr;
    }
    for(auto bb: func->basicBlocks) {
        for(auto instr: bb->instructions) {
            if(instr->reg)
                instr->reg->useHead = nullptr;
        }
    }

    // create new use
    for(auto bb: func->basicBlocks) {
        for(auto instr: bb->instructions) {
            instr->basicblock = bb;
            if(instr->reg)
                instr->reg->I = instr.get();
            int numOperand = instr->getNumOperands();
            if(instr->type == Alloca)
                continue;
            for(int i = 0; i < numOperand; i ++) {
                if(hasUser(instr->type))
                    newUse(instr->getOperand(i).get(), instr.get(), instr->reg.get());
                else
                    newUse(instr->getOperand(i).get(), instr.get());
            }
        }
    }
}
