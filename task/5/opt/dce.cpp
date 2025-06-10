#include "dce.h"


bool findDeadCode(Instruction *Ins, unordered_map<Instruction *, bool>& del){
    //Return,Alloca,Store,Load
    if(Ins->type==Bitcast){
        auto t = ((BitCastInstruction*)(Ins));
        if(t->reg->useHead==nullptr){
            //移除use
            rmInstructionUse(Ins,t->from);
            del[Ins] = true;
            //不是常量且未删除
            if(t->from->I!=nullptr&&del.find(t->from->I) == del.end())
                findDeadCode(t->from->I, del);
            return true;
        }
        return false;
    }
    else if(Ins->type == Ext){
        auto t = ((ExtInstruction*)(Ins));
        if(t->reg->useHead==nullptr){
            //移除use
            rmInstructionUse(Ins,t->from);
            del[Ins] = true;
            if(t->from->I!=nullptr&&del.find(t->from->I) == del.end())
                findDeadCode(t->from->I, del);
            return true;
        }
        return false;
    }
    else if(Ins->type == Sitofp){
        auto t = ((SitofpInstruction*)(Ins));
        if(t->reg->useHead==nullptr){
            //移除use
            rmInstructionUse(Ins,t->from);
            del[Ins] = true;
            if(t->from->I!=nullptr&&del.find(t->from->I) == del.end())
                findDeadCode(t->from->I, del);
            return true;
        }
        return false;
    }
    else if(Ins->type == Fptosi){
        auto t = ((FptosiInstruction*)(Ins));
        if(t->reg->useHead==nullptr){
            //移除use
            rmInstructionUse(Ins,t->from);
            del[Ins] = true;
            if(t->from->I!=nullptr&&del.find(t->from->I) == del.end())
                findDeadCode(t->from->I, del);
            return true;
        }
        return false;
    }
    else if(Ins->type == Call){
        auto t = ((CallInstruction*)(Ins));
        if(t->reg->useHead==nullptr&&t->func->isReenterable&&!(t->func->isLib)){
            del[Ins] = true;
            for(int i=0;i<t->argv.size();i++){
                rmInstructionUse(Ins,t->argv[i]);
                if(t->argv[i]->I!=nullptr&&del.find(t->argv[i]->I) == del.end())
                    findDeadCode(t->argv[i]->I, del);
            }
            return true;
        }
        return false;
    }
    else if(Ins->type == GEP){
        auto t = ((GetElementPtrInstruction*)(Ins));
        if(t->reg->useHead==nullptr){
            del[Ins] = true;
            //移除use
            rmInstructionUse(Ins,t->from);
            if(t->from->I!=nullptr&&del.find(t->from->I) == del.end())
                findDeadCode(t->from->I, del);
            for(int i=0;i<t->index.size();i++){
                rmInstructionUse(Ins,t->index[i]);
                if(t->index[i]->I!=nullptr&&del.find(t->index[i]->I) == del.end())
                    findDeadCode(t->index[i]->I, del);
            }
            return true;
        }
        return false;
    }
    else if(Ins->type == Binary){
        auto t = ((BinaryInstruction*)(Ins));
        if(t->reg->useHead==nullptr){
            del[Ins] = true;
            //移除use
            rmInstructionUse(Ins,t->a);
            if(t->a->I!=nullptr&&del.find(t->a->I) == del.end())
                findDeadCode(t->a->I, del);
            rmInstructionUse(Ins,t->b);
            if(t->b->I!=nullptr&&del.find(t->b->I) == del.end())
                findDeadCode(t->b->I, del);
            return true;
        }
        return false;
    }
    else if(Ins->type == Fneg){
        auto t = ((FnegInstruction*)(Ins));
        if(t->reg->useHead==nullptr){
            //移除use
            rmInstructionUse(Ins,t->a);
            del[Ins] = true;
            if(t->a->I!=nullptr&&del.find(t->a->I) == del.end())
                findDeadCode(t->a->I, del);
            return true;
        }
        return false;
    }
    else if(Ins->type == Icmp){
        auto t = ((IcmpInstruction*)(Ins));
        if(t->reg->useHead==nullptr){
            del[Ins] = true;
            //移除use
            rmInstructionUse(Ins,t->a);
            if(t->a->I!=nullptr&&del.find(t->a->I) == del.end())
                findDeadCode(t->a->I, del);
            rmInstructionUse(Ins,t->b);
            if(t->b->I!=nullptr&&del.find(t->b->I) == del.end())
                findDeadCode(t->b->I, del);
            return true;
        }
        return false;
    }
    else if(Ins->type == Fcmp){
        auto t = ((FcmpInstruction*)(Ins));
        if(t->reg->useHead==nullptr){
            del[Ins] = true;
            //移除use
            rmInstructionUse(Ins,t->a);
            if(t->a->I!=nullptr&&del.find(t->a->I) == del.end())
                findDeadCode(t->a->I, del);
            rmInstructionUse(Ins,t->b);
            if(t->b->I!=nullptr&&del.find(t->b->I) == del.end())
                findDeadCode(t->b->I, del);
            return true;
        }
        return false;
    }
    else if(Ins->type == Phi){
        auto t = ((PhiInstruction*)(Ins));
        if(t->reg->useHead==nullptr){
            del[Ins] = true;
            //移除use
            for(int i=0;i<t->from.size();i++){
                rmInstructionUse(Ins,t->from[i].first);
                if(t->from[i].first->I!=nullptr&&del.find(t->from[i].first->I) == del.end())
                    findDeadCode(t->from[i].first->I, del);
            }
            return true;
        }
        return false;
    }
    return false;
}

void dce(FunctionPtr func){
    unordered_map<Instruction *,bool> del;
    for(auto&bb:(func->basicBlocks)){
        for(auto& ins:(bb->instructions)){
            findDeadCode(ins.get(),del);
        }
    }
    vector<InstructionPtr> newIns;
    for(auto& bb:(func->basicBlocks)){
        newIns.clear();
        for(auto ins:(bb->instructions)){
            if(del.find(ins.get())==del.end()){
                newIns.emplace_back(ins);
            }
        }
        bb->instructions = newIns;
    }
}