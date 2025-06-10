#ifndef REASSOCIATE_HPP
#define REASSOCIATE_HPP

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

//指令重排
// a = b + c ; a = a + c -> a = b + 2 * c



void reassociate(FunctionPtr func);

#endif