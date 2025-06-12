#pragma once
#include "BasicBlock.h"
#include "Instruction.h"
#include "Function.h"

void addEdgeInCFG(shared_ptr<BasicBlock> pred,shared_ptr<BasicBlock> next);
void computeCFG(FunctionPtr func);
