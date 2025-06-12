#pragma once

#include "Variable.h"
#include "Function.h"
#include "Instruction.h"

bool hasUser(InsID id);
void computeUse(FunctionPtr func);
