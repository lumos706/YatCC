#include "reg2mem.h"


void reg2mem(FunctionPtr func){
    vector<VariablePtr> newAlloca;
    vector<BasicBlockPtr> addBB;
    unordered_map<string,bool> used;

    for(auto &bb:(func->basicBlocks)){
        for(auto &ins:(bb->instructions)){
            if(ins->type == Phi){
                //在entry分配空间
                auto t = (PhiInstruction*)(ins.get());
                VariablePtr alloc;
                if(t->reg->type->isInt()){
                    alloc = VariablePtr(new Int(t->reg->name+".addr", false, false));
                }
                else if(t->reg->type->isFloat()){
                    alloc = VariablePtr(new Float(t->reg->name+".addr", false, false));
                }
                else{
                    alloc = VariablePtr(new Ptr(t->reg->name+".addr", false, false, t->reg->type));
                }
                newAlloca.emplace_back(alloc);
                for(auto &coming:t->from){
                    auto newStore = InstructionPtr(new StoreInstruction(alloc, coming.first, coming.second));
                    coming.second->instructions.emplace(coming.second->instructions.end()-1, newStore);

                    rmInstructionUse(ins, coming.first);
                }
                auto newLoad  = shared_ptr<LoadInstruction>(new LoadInstruction(alloc, t->reg, bb));
                ins = newLoad;
            }
        }
    }
    //在entry分配空间
    auto tmp = func->basicBlocks[0]->instructions;
    func->basicBlocks[0]->instructions.clear();
    for (auto &var : newAlloca) func->basicBlocks[0]->instructions.emplace_back(InstructionPtr(new AllocaInstruction(var, func->basicBlocks[0])));
    for(auto &ins: tmp) func->basicBlocks[0]->instructions.emplace_back(ins);
}