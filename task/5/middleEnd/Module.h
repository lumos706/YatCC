#pragma once

#include<stack>
#include"Function.h"
#include"StringTable.h"

using std::stack;

struct Module
{
    // 所有全局函数
    vector<FunctionPtr> globalFunctions;
    // 所有全局变量
    vector<VariablePtr> globalVariables;
    // 变量表
    StringTableNodePtr globalStringTable;
    StringTableNodePtr currStringTable;
    StringTableNodePtr paramStringTable;
    TypePtr declType;
    // 辅助前端的栈结构
    stack<LabelPtr> trueLabelStack;
    stack<LabelPtr> falseLabelStack;
    stack<LabelPtr> whileEndStack;
    stack<LabelPtr> whileCondStack;
    stack<FunctionPtr> funcStack;
    Module();
    
    // 添加函数、变量和block
    void pushGlobalFunction(FunctionPtr Function);
    void pushFunc(FunctionPtr func);
    // 从blockStack弹出block
    FunctionPtr popFunc();
    void pushVariable(VariablePtr globalVariable);
    // 压入stringTable
    void pushBackStringTable();
    // 从stringTable弹出stringTable
    StringTableNodePtr popStringTable();
    // 获取栈顶block
    FunctionPtr getFunc();
    // 获得栈顶block中最后一个basicblock
    shared_ptr<BasicBlock> getBasicBlock();
    FunctionPtr getFunction(string name);

    void registerVariable(VariablePtr variable);
    bool blockStackEmpty();
    void print();
};
