#pragma once

#include <unordered_map>
#include <numeric>
#include <queue>
#include "Module.h"
#include "Loop.h"

bool isCallIdempotent(CallInstruction *ins);
void LICM(FunctionPtr func);
bool isLoopInvariant(InstructionPtr instr, LoopPtr curLoop, FunctionPtr func, set<Instruction *>& toDelete);