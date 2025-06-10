#ifndef INLINER_HPP
#define INLINER_HPP

#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <stack>
#include <set>
#include <climits>
#include <map>
#include <algorithm>

#include "Module.h"

using namespace std;


static VariablePtr copyVariable(VariablePtr old);

static FunctionPtr copyFunction(FunctionPtr func, Module& ir, int callNum);

void inliner(Module& ir);

#endif