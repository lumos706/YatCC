#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include "StringTable.h"
#include "Label.h"
#include "Loop.h"

struct Loop;
struct BasicBlock;
struct Instruction;


struct Function:public std::enable_shared_from_this<Function>
{
    // 函数返回值
    ValuePtr retVal;
    // return指令所在块
    shared_ptr<BasicBlock> returnBasicBlock;
    // 函数名
    string name;
    // 形参
    vector<ValuePtr> formArguments;

    // 是否是库函数
    bool isLib;
    bool isReenterable;

    //记录该函数的调用者以及调用次数
    unordered_map<shared_ptr<Function>,int> caller;
    //记录该函数调用的函数以及调用次数
    unordered_map<shared_ptr<Function>,int> callee;
    // 记录调用该函数的ins
    unordered_set<shared_ptr<Instruction>> callerIns;

    Function();
    Function(TypePtr returnType, string name, vector<ValuePtr> formArguments);
    Function(TypePtr returnType, string name, bool isReenterable, vector<ValuePtr> formArguments=vector<ValuePtr>()) : retVal{ValuePtr(new Reg(returnType, "retval"))}, name{name}, formArguments{formArguments}, isLib{true} , isReenterable{isReenterable} {};
    // 设置returnBasicBlock
    void setReturnBasicBlock();
    void solveReturnBasicBlock();

    string getTypeStr() { return retVal->type->getStr() + " @" + name; }
    void print();

    // from Blcok
    int regNum;
    ValuePtr getReg(TypePtr type){return ValuePtr(new Reg(type, regNum++));}
    // 用于装载基本块
    vector<shared_ptr<BasicBlock>> basicBlocks;
    // 每一个variables有一个名字，通过map将其联系起来
    std::unordered_map<string, VariablePtr> variables;
    // 每一个basicblock有一个label，通过map将其联系起来
    unordered_map<LabelPtr, shared_ptr<BasicBlock>> LabelBBMap;
    void getLabelBBMap();

    // loops
    vector<shared_ptr<Loop>> loops;
    unordered_map<shared_ptr<BasicBlock>, shared_ptr<Loop>> bbToLoop;
    vector<shared_ptr<Loop>> topLoops;
    vector<shared_ptr<Loop>> getAllLoops() { return loops; }
    vector<shared_ptr<Loop>> getLoopsInPostorder();
    vector<shared_ptr<Loop>> getTopLoops() { return topLoops; }
    void addLoop(shared_ptr<Loop> loop) { loops.push_back(loop); }
    void addTopLoop(shared_ptr<Loop> loop) { topLoops.push_back(loop); }
    void addBBToLoop(shared_ptr<BasicBlock> bb, shared_ptr<Loop> loop)  { bbToLoop[bb] = loop; }
    shared_ptr<Loop> getLoopOfBB(shared_ptr<BasicBlock> bb) { return bbToLoop.count(bb)? bbToLoop[bb]: nullptr; }
    void clearBBToLoop();
    void clearLoops();

    // 设置Block所属的函数
    void setBBbelongFunc(shared_ptr<Function> func);
    // 添加一个basicblock
    void pushBasicBlock(shared_ptr<BasicBlock> basicblock);
    void pushVariable(VariablePtr variable);
    // 通过名字查找变量
    VariablePtr findVariable(string name);
    // 为本地变量分配空间，即在指令前插入alloca指令
    void allocLocalVariable();
    // 将instructions在end指令处分割
    void solveEndInstruction();
    // 获取第一个基本块
    shared_ptr<BasicBlock> getEntryBlock();
};
typedef shared_ptr<Function> FunctionPtr;