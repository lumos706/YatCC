#include "cse.h"

bool impossibleToCSE(InsID type) {
    return (type == Br || type == Return || type == Alloca || type == Ext 
        || type == Phi || type == Icmp || type == Fcmp);
}

bool isMemoryRelated(InsID type) {
    return type == Load || type == Store || type == Call;
}

void cse(FunctionPtr func){
    int instrCount = 0;
    for(int bbIdx = 0; bbIdx < func->basicBlocks.size(); bbIdx ++) {
        auto bb = func->basicBlocks[bbIdx];
        unordered_map<InstructionPtr, BasicBlockPtr> del;
        vector<BasicBlockPtr> succBlocks{bb};
        unordered_set<BasicBlockPtr> changedBB;
        unordered_map<BrInstruction *, bool> canBrContinue;
        unordered_map<BasicBlockPtr, bool> canBBContinue;
        for(auto succBB: bb->succBasicBlocks) {
            succBlocks.emplace_back(succBB);
        }

        for(int i = 0; i < bb->instructions.size(); i++){
            //此时是ssa，所以不需要考虑值会不会改变
            auto Ia = bb->instructions[i];
            bool flag = false;

            // impossible to do CSE
            if(impossibleToCSE(Ia->type) || del.count(Ia))
                continue;

            for(auto succBB: succBlocks) {
                int j = (succBB == bb ? i + 1 : 0);

                // if CSE for succBB may incur error, skip it
                if(canBBContinue.find(succBB) != canBBContinue.end() && !canBBContinue[succBB])
                    continue;
    
                for( ; j < succBB->instructions.size(); j++){
                    auto Ib = succBB->instructions[j];
                    // load/store -> call
                    if((Ia->type == Load || Ia->type == Store) && Ib->type == Call) {
                        // if happened in current block, it dominates the other blocks
                        if(succBB == bb)
                            flag = true;
                        else
                            break;
                    }
                    // destination of Br
                    if(Ib->type == Br) {
                        auto br = (BrInstruction *)(Ib.get());
                        bool canContinue;
                        if(canBrContinue.find(br) != canBrContinue.end()) {
                            canContinue = canBrContinue[br];
                        }
                        else {
                            // determinate whether succBB is reached by 'bb' only
                            auto nextBB = func->LabelBBMap[br->label_true];
                            canBBContinue[nextBB] = (nextBB->predBasicBlocks.size() == 1);
                            canContinue = canBBContinue[nextBB];
                            if(br->label_false) {
                                nextBB = func->LabelBBMap[br->label_false];
                                canBBContinue[nextBB] = (nextBB->predBasicBlocks.size() == 1);
                                canContinue = canContinue || canBBContinue[nextBB];
                            }
                            canBrContinue[br] = canContinue;
                        }
                        if(!canContinue) {
                            if(succBB == bb)
                                flag = true;
                            else
                                break;
                        }
                    }
                    if(flag) 
                        break;

                    if(del.count(Ib)) 
                        continue;

                    if(Ia->type == Ib->type){
                        if(Ia->type == Binary){
                            auto Ii = (BinaryInstruction*)(Ia.get());
                            auto Ij = (BinaryInstruction*)(Ib.get());
                            if(Ii->op!=Ij->op){
                                continue;
                            }
                            if(Ii->a==Ij->a&&Ii->b==Ij->b){
                                replaceVarByVar(Ij->reg, Ii->reg);
                                del[Ib] = succBB;
                                changedBB.insert(succBB);
                                deleteUser(Ij->reg);
                            }
                            else if(Ii->a->isConst&&Ij->a->isConst&&Ii->b==Ij->b){
                                auto newA1 = dynamic_cast<Const*>(Ii->a.get());
                                auto newA2 = dynamic_cast<Const*>(Ij->a.get());
                                if(newA1->type->isInt()&&newA2->type->isInt()){
                                    if(newA1->intVal ==  newA2->intVal){
                                        replaceVarByVar(Ij->reg, Ii->reg);
                                        del[Ib] = succBB;
                                        changedBB.insert(succBB);
                                        deleteUser(Ij->reg);
                                    }
                                }
                                else if(newA1->type->isFloat()&&newA2->type->isFloat()){
                                    if(newA1->floatVal ==  newA2->floatVal){
                                        replaceVarByVar(Ij->reg, Ii->reg);
                                        del[Ib] = succBB;
                                        changedBB.insert(succBB);
                                        deleteUser(Ij->reg);
                                    }
                                }
                                else if(newA1->type->isBool()&&newA2->type->isBool()){
                                    if(newA1->boolVal ==  newA2->boolVal){
                                        replaceVarByVar(Ij->reg, Ii->reg);
                                        del[Ib] = succBB;
                                        changedBB.insert(succBB);
                                        deleteUser(Ij->reg);
                                    }
                                }
                            }
                            else if(Ii->b->isConst&&Ij->b->isConst&&Ii->a==Ij->a){
                                auto newB1 = dynamic_cast<Const*>(Ii->b.get());
                                auto newB2 = dynamic_cast<Const*>(Ij->b.get());
                                if(newB1->type->isInt()&&newB2->type->isInt()){
                                    if(newB1->intVal ==  newB2->intVal){
                                        replaceVarByVar(Ij->reg, Ii->reg);
                                        del[Ib] = succBB;
                                        changedBB.insert(succBB);
                                        deleteUser(Ij->reg);
                                    }
                                }
                                else if(newB1->type->isFloat()&&newB2->type->isFloat()){
                                    if(newB1->floatVal ==  newB2->floatVal){
                                        replaceVarByVar(Ij->reg, Ii->reg);
                                        del[Ib] = succBB;
                                        changedBB.insert(succBB);
                                        deleteUser(Ij->reg);
                                    }
                                }
                                else if(newB1->type->isBool()&&newB2->type->isBool()){
                                    if(newB1->boolVal ==  newB2->boolVal){
                                        replaceVarByVar(Ij->reg, Ii->reg);
                                        del[Ib] = succBB;
                                        changedBB.insert(succBB);
                                        deleteUser(Ij->reg);
                                    }
                                }
                            }
                            //两个都是常量
                            else{
                            }
                        }
                        // store -> store
                        else if(Ia->type == Store) {
                            auto Ii = (StoreInstruction*)(Ia.get());
                            auto Ij = (StoreInstruction*)(Ib.get());
                            if(Ii->des == Ij->des) {
                                if(succBB == bb) 
                                    flag = true;
                                else
                                    break;
                            }
                            if(Ii->des == Ij->des) {
                                del[Ia] = bb;
                                changedBB.insert(bb);
                                rmInstructionUse(Ia, Ii->des);
                                rmInstructionUse(Ia, Ii->value);
                            }
                        }
                        // load -> load
                        else if(Ia->type == Load) {
                            auto Ii = (LoadInstruction*)(Ia.get());
                            auto Ij = (LoadInstruction*)(Ib.get());
                            if(Ii->from == Ij->from) {
                                replaceVarByVar(Ij->to, Ii->to);
                                del[Ib] = succBB;
                                changedBB.insert(succBB);
                                deleteUser(Ij->to);
                            }
                        }
                        // pointer alias
                        else if(Ia->type == GEP) {
                            auto Ii = (GetElementPtrInstruction*)(Ia.get());
                            auto Ij = (GetElementPtrInstruction*)(Ib.get());
                            if (Ii->from == Ij->from && Ii->getNumOperands() == Ij->getNumOperands()) {
                                bool matched = true;
                                for(int i = 0; i < Ii->index.size(); i ++) {
                                    // same SSA variable
                                    if(Ii->index[i] == Ij->index[i])
                                        continue;
                                    
                                    // same constant
                                    auto const0 = dynamic_cast<Const *>(Ii->index[i].get());
                                    auto const1 = dynamic_cast<Const *>(Ij->index[i].get());
                                    if(const0 == nullptr || const1 == nullptr || const0->intVal != const1->intVal) {
                                        matched = false;
                                        break;
                                    }
                                }
                                if(matched) {
                                    replaceVarByVar(Ij->reg, Ii->reg);
                                    del[Ib] = succBB;
                                    changedBB.insert(succBB);
                                    deleteUser(Ij->reg);
                                }
                            }
                        }
                    }
                    else if(Ia->type == Store || Ia->type == Load) {
                        // store -> load
                        if(Ia->type == Store && Ib->type == Load) {
                            auto Ii = (StoreInstruction*)(Ia.get());
                            auto Ij = (LoadInstruction*)(Ib.get());
                            if(Ii->des == Ij->from) {
                                replaceVarByVar(Ij->to, Ii->value);
                                del[Ib] = succBB;
                                changedBB.insert(succBB);
                                deleteUser(Ij->to);
                            }
                        }
                        // load -> store
                        if(Ia->type == Load && Ib->type == Store) {
                            auto Ii = (LoadInstruction*)(Ia.get());
                            auto Ij = (StoreInstruction*)(Ib.get());
                            if(Ii->from == Ij->des) {
                                flag = true;
                            }
                        }
                    }
                }
            }
        }
        for(auto newBB: changedBB) {
            vector<InstructionPtr > newIns;
            for(int i = 0; i < newBB->instructions.size(); i++){
                if(del.count(newBB->instructions[i]) == 0){
                    newIns.push_back(newBB->instructions[i]);
                }
            }
            newBB->instructions = newIns;
        }
        instrCount += del.size();

    }
}