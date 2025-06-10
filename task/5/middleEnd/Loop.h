#pragma once

#include <set>
#include <vector>
#include "Instruction.h"
#include "BasicBlock.h"
#include "utils.h"
using namespace std;

struct BasicBlock;
struct Function;
struct Instruction;
struct PhiInstruction;
struct BinaryInstruction;
struct Value;

class SCEV
{
private:
    vector<ValuePtr> scevVal;
    set<BinaryInstruction*> instructionsHasBeenCaculated;
public:
    SCEV() {};
    SCEV(ValuePtr initial, ValuePtr step);
    SCEV(ValuePtr initial, const SCEV& innerSCEV);
    SCEV(const vector<ValuePtr>& vec);
    SCEV(ValuePtr initial, ValuePtr step, std::set<BinaryInstruction*>&& binarys);

    friend SCEV operator+(ValuePtr lhs, const SCEV& rhs);
    friend SCEV operator+(const SCEV& lhs, const SCEV& rhs);

    friend SCEV operator-(const SCEV& lhs, ValuePtr rhs);
    friend SCEV operator-(const SCEV& lhs, const SCEV& rhs);

    friend SCEV operator*(ValuePtr lhs, const SCEV& rhs);

    ValuePtr& at(unsigned i) { return scevVal.at(i); }

    size_t size() { return scevVal.size(); }

    ValuePtr at(unsigned i) const { return scevVal.at(i); }

    size_t size() const { return scevVal.size(); }

    void getItemChains(unsigned i, std::vector<BinaryInstruction*>& instrChain);

    string getSignature();

    void clear();
};
typedef shared_ptr<SCEV> SCEVPtr;

struct Loop
{
    set<shared_ptr<BasicBlock>> bbSet;
    vector<shared_ptr<BasicBlock>> basicBlocks;
    shared_ptr<BasicBlock> header = nullptr;
    shared_ptr<BasicBlock> preHeader = nullptr;
    shared_ptr<BasicBlock> latchBlock = nullptr;
    // set<shared_ptr<Loop>> innerLoops;
    shared_ptr<Loop> parentLoop = nullptr;
    vector<shared_ptr<Loop>> subLoops;
    unordered_set<shared_ptr<BasicBlock>> exitingBlocks; 
    unordered_set<shared_ptr<BasicBlock>> exitBlocks;
    shared_ptr<Function> parentFunc = nullptr;
    shared_ptr<Function> parent = nullptr;

    unordered_map<Instruction*, SCEV> SCEVCheck;

    int id;
    int loopDepth;
    bool isInner;

    // SCEV
    // std::unordered_map<Instruction*, SCEV> SCEVCheck;

    PhiInstruction *indPhi;
    shared_ptr<Instruction> indCondVar;
    shared_ptr<Value> indEnd;
    IcmpKind icmpKind;
    int tripCount;

    Loop() {}
    Loop(shared_ptr<BasicBlock> header, int id);

    bool contains(shared_ptr<BasicBlock> bb) { return bbSet.count(bb); }

    shared_ptr<BasicBlock> getHeader()                   { return header; }
    shared_ptr<BasicBlock> getPreheader()                { return preHeader; }
    shared_ptr<Function> getFunc()                         { return parentFunc; }
    set<shared_ptr<BasicBlock>> getBasicBlocks()         { return bbSet; }
    vector<shared_ptr<BasicBlock>> getLoopBasicBlocks()  { return basicBlocks; }
    shared_ptr<BasicBlock> getLatchBlock()               { return latchBlock; }
    shared_ptr<Loop> getParentLoop()            { return parentLoop; }
    vector<shared_ptr<Loop>> getSubLoops()      { return subLoops; }
    unordered_set<shared_ptr<BasicBlock>> getExitingBlocks() { return exitingBlocks; }
    unordered_set<shared_ptr<BasicBlock>> getExitBlocks()    { return exitBlocks; }
    vector<shared_ptr<BasicBlock>>& getLoopBasicBlocksRef()  { return basicBlocks; }
    vector<shared_ptr<Loop>>& getSubLoopsRef()      { return subLoops; }
    int getLoopDepth() { return loopDepth; }

    IcmpKind getIcmpKind() { return icmpKind; }
    int getTripCount() { return tripCount;}
    void setLoopBasicBlocks(vector<shared_ptr<BasicBlock>> bbs) { basicBlocks = bbs; }
    void setPreheader(shared_ptr<BasicBlock> _preHeader)     { preHeader = _preHeader; }
    void setLatchBlock(shared_ptr<BasicBlock> bb)            { latchBlock = bb; }
    void setParentLoop(shared_ptr<Loop> parent)     { parentLoop = parent; }
    void setSubLoops(vector<shared_ptr<Loop>> loops) { subLoops = loops; }
    
    // maintain basicBlocks and bbSets
    void addBasicBlock(shared_ptr<BasicBlock> bb)        { basicBlocks.push_back(bb); bbSet.insert(bb);}
    void addSubLoop(shared_ptr<Loop> loop)      { subLoops.push_back(loop); }
    void addExitingBlocks(shared_ptr<BasicBlock> bb)     { exitingBlocks.insert(bb); }
    void addExitBlocks(shared_ptr<BasicBlock> bb)        { exitBlocks.insert(bb); }
    void setLoopDepth(int depth)                { loopDepth = depth; }
    void setIndexCondInstr(shared_ptr<Instruction> instr) { indCondVar = instr; }
    void setICmpKind(IcmpKind kind) { icmpKind = kind; }
    void setIndexEnd(ValuePtr ind)              { indEnd = ind; }
    void setIndexPhi(PhiInstruction *phi)       { indPhi = phi; }

    SCEV& getSCEV(Instruction* instr);

    bool hasSCEV(Instruction* instr);

    void registerSCEV(Instruction* instr, SCEV scev);

    void cleanSCEV() { SCEVCheck.clear(); }

    static bool isADominatorB(shared_ptr<BasicBlock> A, shared_ptr<BasicBlock> B, shared_ptr<BasicBlock> entry);
    bool isSimpleLoopInvariant(ValuePtr value);
    bool isInnerLoop() { return parentLoop != nullptr; }
};
typedef shared_ptr<Loop> LoopPtr;

