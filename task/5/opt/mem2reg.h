#include <iostream>
#include <vector>
#include <unordered_map>
#include <queue>
#include <stack>
#include "Module.h"

using namespace std;
struct node{
    shared_ptr<BasicBlock> bb;
    vector<node*> son;
    node* father = nullptr;
    node(shared_ptr<BasicBlock> ibb):bb{ibb}{};
};


void domTree(FunctionPtr func);
void mem2reg(FunctionPtr func);