#include "Function.h"

void Function::setReturnBasicBlock(){
    returnBasicBlock = BasicBlockPtr(new BasicBlock(LabelPtr(new Label("return"))));
}

Function::Function() : regNum(0) {
    basicBlocks.emplace_back(std::make_shared<BasicBlock>());
}

Function::Function(TypePtr returnType, string name, vector<ValuePtr> formArguments) : retVal{ValuePtr(new Reg(returnType, "retval"))}, name{name}, formArguments{formArguments}, isLib{false}, isReenterable{true}
{
    basicBlocks.emplace_back(std::make_shared<BasicBlock>());
    regNum = 0;
};

void Function::solveReturnBasicBlock()
{
    if (retVal->type->isVoid())
    {
        returnBasicBlock->setEndInstruction(InstructionPtr(new ReturnInstruction(retVal, returnBasicBlock)));
        pushBasicBlock(returnBasicBlock);
    }
    else
    {
        auto ins = shared_ptr<LoadInstruction>(new LoadInstruction(retVal, getReg(retVal->type), returnBasicBlock));
        returnBasicBlock->pushInstruction(ins);
        returnBasicBlock->setEndInstruction(InstructionPtr(new ReturnInstruction(ins->to, returnBasicBlock)));
        pushBasicBlock(returnBasicBlock);
    }
}

void Function::print()
{
    if (isLib)
    {
        cout << "declare " << getTypeStr() << "(";
        for (int i = 0; i < formArguments.size(); i++)
        {
            if (i)
                cout << ", ";
            cout << formArguments[i]->type->getStr();
        }
        cout << ")" << endl;
    }
    else
    {
        cout << "define " << retVal->type->getStr() << " @" << this->name << "(";
        for (int i = 0; i < formArguments.size(); i++)
        {
            if (i)
                cout << ", ";
            cout << formArguments[i]->getTypeStr();
        }
        cout << ") {" << endl;
        for (int i = 0; i < basicBlocks.size(); i++)
        {
            basicBlocks[i]->print();
            if (i != basicBlocks.size() - 1)
                cout << endl;
        }
        cout << "}" << endl
             << endl;
    }
}

void Function::setBBbelongFunc(shared_ptr<Function> func){
    for(auto bb:basicBlocks){
        bb->belongfunc = func;
    }
}

void Function::getLabelBBMap(){
    for(int i=0;i<basicBlocks.size();i++){
        LabelBBMap[basicBlocks[i]->label] = basicBlocks[i];
    }
}

void Function::pushVariable(VariablePtr variable)
{
    variables[variable->name]= variable;
}

VariablePtr Function::findVariable(string name)
{
    if (variables.find(name)!=variables.end()){
        return variables[name];
    }

    return nullptr;
}

void Function::allocLocalVariable()
{
    auto tmp = basicBlocks[0]->instructions;
    basicBlocks[0]->instructions.clear();
    for (auto &var : variables) basicBlocks[0]->instructions.emplace_back(InstructionPtr(new AllocaInstruction(var.second, basicBlocks[0])));
    for(auto &ins: tmp) basicBlocks[0]->instructions.emplace_back(ins);
}

void Function::solveEndInstruction()
{
    for (auto &basicBlock : basicBlocks)
    {
        vector<shared_ptr<Instruction>> newIns;
        int i  =0;
        for(;i<basicBlock->instructions.size();i++){
            // cerr<<i<<"  "<<basicBlock->instructions[i]->type<<endl;
            newIns.push_back(basicBlock->instructions[i]);
            if(basicBlock->instructions[i]->type==Store&&dynamic_cast<StoreInstruction*>(basicBlock->instructions[i].get())->des->name=="retval"){
                break;
            }
            
        }
        basicBlock->instructions = newIns;

        assert(basicBlock->endInstruction);
        basicBlock->instructions.emplace_back(basicBlock->endInstruction);
    }
}

void Function::pushBasicBlock(shared_ptr<BasicBlock> basicblock)
{
    basicBlocks.emplace_back(basicblock);
}


void Function::clearBBToLoop() {
    bbToLoop.clear();
    for(auto basicBlock: basicBlocks) {
        basicBlock->loop = nullptr;
        basicBlock->loopDepth = 0;
    }
}

void Function::clearLoops() {
    loops.clear();
}

BasicBlockPtr Function::getEntryBlock() {
    assert(basicBlocks.size() > 0);
    return basicBlocks[0];
}


void preorderVisit(LoopPtr loop, set<LoopPtr>& visited, vector<LoopPtr>& preorder, int& loopCnt) {
    visited.insert(loop);
    for(auto innerLoop: loop->getSubLoops()) {
        preorderVisit(innerLoop, visited, preorder, loopCnt);
    }
    preorder[loopCnt ++] = loop;
}

// preorder traverse loops and their subloops
vector<LoopPtr> Function::getLoopsInPostorder() {
    set<LoopPtr> visited;
    vector<LoopPtr> preorder(loops.size());
    int loopCnt = 0;
    for(auto loop: loops) {
        if(visited.find(loop) != visited.end()) continue;
        preorderVisit(loop, visited, preorder, loopCnt);
    }
    assert(loopCnt == loops.size());
    return preorder;
}
