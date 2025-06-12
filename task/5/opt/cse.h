//公共子表达式删除
#pragma once
#include <iostream>
#include <vector>
#include <unordered_map>
#include <queue>
#include <stack>
#include "Module.h"

bool impossibleToCSE(InsID type);
bool isMemoryRelated(InsID type);
void cse(FunctionPtr func);