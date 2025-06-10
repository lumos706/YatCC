#pragma once
#include <unordered_set>
#include <iostream>
#include <memory>
#include <functional>
#include "Instruction.h"
#include "Label.h"
#include <algorithm>
#include <map>

using namespace std;

struct Instruction;
struct Function;
struct Loop;


struct BasicBlock
{
    // 每一个basicblock有一个label
    LabelPtr label;
    // 该 basicblock 的指令
    vector<shared_ptr<Instruction>> instructions;
    // 该 basicblock 的endins
    shared_ptr<Instruction> endInstruction= nullptr;
    // 所属的func
    shared_ptr<Function> belongfunc;

    BasicBlock(LabelPtr label=LabelPtr(new Label())): label{label} ,belongfunc{nullptr}{};
    void setBelongFunc(shared_ptr<Function> func){
        belongfunc = func;
    }
    shared_ptr<Function> getParent() { return belongfunc; }

    // 在指令集末尾插入指令
    void pushInstruction(shared_ptr<Instruction> instruction);
    void pushInstruction(vector<shared_ptr<Instruction>> toInsert);
    // 在next指令前插入指令
    void insertInstruction(shared_ptr<Instruction> instruction, shared_ptr<Instruction> next);
    // 设置endinstruction
    void setEndInstruction(shared_ptr<Instruction> instruction);
    void removeInsturction(shared_ptr<Instruction> instruction);

    //用于后续优化阶段的标记
    bool isDeadBB=false;
    bool isWhileCond = false;
    bool isIfCond = false;

    // 循环深度
    int loopDepth = 0;
    shared_ptr<Loop> loop = nullptr;
    static void basicBlockDFSPost(shared_ptr<BasicBlock> bb, function<bool(shared_ptr<BasicBlock>)> cond, function<void(shared_ptr<BasicBlock>)> action);
    static void domTreeDFSPost(shared_ptr<BasicBlock> bb, function<bool(shared_ptr<BasicBlock>)> cond, function<void(shared_ptr<BasicBlock>)> action);

    //前继后继基本块
    unordered_set<shared_ptr<BasicBlock>> predBasicBlocks;
    unordered_set<shared_ptr<BasicBlock>> succBasicBlocks;
    
    //该基本块直接支配的子基本块
    unordered_set<shared_ptr<BasicBlock>> dominatorSon;
    //支配边界
    unordered_set<shared_ptr<BasicBlock>> DF;
    //用来存储直接支配该基本块的基本块
    shared_ptr<BasicBlock> directDominator = nullptr;
    //能够支配的该基本块所有基本块
    unordered_set<shared_ptr<BasicBlock>> allDoms;

    // 添加一个前驱基本块
    void addPredBasicBlock(shared_ptr<BasicBlock> bb)       { predBasicBlocks.insert(bb); }
    // 添加一个后继基本块
    void addSuccBasicBlock(shared_ptr<BasicBlock> bb)       { succBasicBlocks.insert(bb); }
    // 删除一个前驱基本块
    void deletePredBasicBlock(shared_ptr<BasicBlock> bb)    { predBasicBlocks.erase(bb); }
    // 删除一个后继基本块
    void deleteSuccBasicBlock(shared_ptr<BasicBlock> bb)    { succBasicBlocks.erase(bb); }
    // 获取所有前驱基本块
    unordered_set<shared_ptr<BasicBlock>> getPredecessor()  { return predBasicBlocks; }
    // 获取所有后继基本块
    unordered_set<shared_ptr<BasicBlock>> getSuccessor()    { return succBasicBlocks; }
    // 获取入度（前驱基本块的数量）
    int getIndegree()                                       { return predBasicBlocks.size(); }
    // 获取出度（后继基本块的数量）
    int getOutdegree()                                      { return succBasicBlocks.size(); }

    // 维护支配树
    // 添加一个被该基本块直接支配的子基本块
    void addDominatorSon(shared_ptr<BasicBlock> bb)         { dominatorSon.insert(bb); }
    // 添加一个支配边界基本块
    void addDF(shared_ptr<BasicBlock> bb)                   { DF.insert(bb); }
    // 删除一个被该基本块直接支配的子基本块
    void deleteDominatorSon(shared_ptr<BasicBlock> bb)      { dominatorSon.erase(bb); }
    // 删除一个支配边界基本块
    void deleteDF(shared_ptr<BasicBlock> bb)                { DF.erase(bb); }
    // 获取所有被该基本块直接支配的子基本块
    unordered_set<shared_ptr<BasicBlock>> getDominatorSon() { return dominatorSon; }
    // 获取所有支配边界基本块
    unordered_set<shared_ptr<BasicBlock>> getDF()           { return DF; }
    // 设置直接支配该基本块的基本块
    void setDirectDominator(shared_ptr<BasicBlock> bb)      { directDominator = bb; }
    // 设置能够支配该基本块的所有基本块
    void setAllDoms(unordered_set<shared_ptr<BasicBlock>> doms) { allDoms = doms; }
    // 获取直接支配该基本块的基本块
    shared_ptr<BasicBlock> getDirectDominator()             { return directDominator; }
    // 获取能够支配该基本块的所有基本块
    unordered_set<shared_ptr<BasicBlock>> getAllDoms()      { return allDoms; }

    void print();
};
typedef shared_ptr<BasicBlock> BasicBlockPtr;

