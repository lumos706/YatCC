#include "BasicBlock.h"
#include "Loop.h"


void BasicBlock::pushInstruction(InstructionPtr instruction)
{
    if(!endInstruction) instructions.emplace_back(instruction);
    instruction->basicblock = this->instructions[0]->basicblock;
}

void BasicBlock::pushInstruction(vector<InstructionPtr> toInsert)
{
    auto tmpEnd = instructions.empty() ? nullptr : instructions.back();
    vector<InstructionPtr> newInstr(instructions);
    auto bb = instructions[0]->basicblock;

    if(tmpEnd) {
        assert(tmpEnd->type == Br || tmpEnd->type == Return);
        newInstr.pop_back();
        endInstruction = nullptr;
    }
    for(auto instr: toInsert) {
        instr->basicblock = bb;
        newInstr.emplace_back(instr);
    }
    if(tmpEnd) {
        newInstr.emplace_back(tmpEnd);
        endInstruction = tmpEnd;
    }
    instructions = newInstr;
}

// 将instruction 插入到next前
void BasicBlock::insertInstruction(InstructionPtr instruction, InstructionPtr next)
{
    auto curInstruction = instructions.begin();
    while (curInstruction != instructions.end())
    {
        if (*curInstruction == next) break;
        curInstruction++;
    }
    // make sure next can be found
    assert(curInstruction != instructions.end());
    instructions.emplace(curInstruction, instruction);
    // instruction->basicblock = this->instructions[0]->basicblock;
    instruction->basicblock = next->basicblock;
}

void BasicBlock::removeInsturction(InstructionPtr instruction)
{
    assert(instruction != endInstruction);
    auto curInstruction = instructions.begin();
    int time = 0;
    while (curInstruction != instructions.end())
    {
        // cerr << time++ << endl;
        if (*curInstruction == instruction) break;
        curInstruction++;
    }
    // cerr << "[passed] loop" << endl;
    // make sure next can be found
    assert(curInstruction != instructions.end());
    instructions.erase(curInstruction);
}

void BasicBlock::setEndInstruction(InstructionPtr instruction)
{
    if(!endInstruction) endInstruction = instruction;
}

void BasicBlock::print()
{
    label->print();

    int cnt = 0,n=predBasicBlocks.size();
    for(auto &pred : predBasicBlocks){
        if(cnt==0){
            cout<<"\t\t\t; preds = ";
        }
        cout<<"%"+pred->label->name;
        if(cnt<n-1){
            cout<<", ";
        }
        cnt++;
    }
    cout<<endl;
    for (auto &instruction : instructions)
    {
        instruction->print();
    }
}

void BasicBlock::basicBlockDFSPost(shared_ptr<BasicBlock> bb, function<bool(shared_ptr<BasicBlock>)> cond, function<void(shared_ptr<BasicBlock>)> action) {
    if(cond(bb))
        return;
    for(auto succ : bb->getSuccessor())
        basicBlockDFSPost(succ, cond, action);
    action(bb);
}

void BasicBlock::domTreeDFSPost(shared_ptr<BasicBlock> bb, function<bool(shared_ptr<BasicBlock>)> cond, function<void(shared_ptr<BasicBlock>)> action) {
    if(cond(bb))
        return;
    for(auto succ : bb->getDominatorSon())
        domTreeDFSPost(succ, cond, action);
    action(bb);
}
