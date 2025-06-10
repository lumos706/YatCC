#include "simplifyCFG.h"


bool removeUnreachableBlocks(FunctionPtr func){
    vector<BasicBlockPtr> newBB;
    bool change = false;
    for(int i = 0;i<func->basicBlocks.size();i++){
        if(i==0||!func->basicBlocks[i]->predBasicBlocks.empty()){
            newBB.push_back(func->basicBlocks[i]);
        }
        else{
            for(auto succ:func->basicBlocks[i]->succBasicBlocks){
                succ->predBasicBlocks.erase(func->basicBlocks[i]);
            }
            change = true;
        }
    }
    func->basicBlocks = newBB;

    for(auto bb:func->basicBlocks){
        for(auto it = bb->instructions.begin();it!=bb->instructions.end();){
            if((*it)->type == Phi){
                auto Pi = dynamic_cast<PhiInstruction*>((*it).get());
                for(auto fit = Pi->from.begin();fit!=Pi->from.end();){
                    if(bb->predBasicBlocks.find(fit->second)==bb->predBasicBlocks.end()){
                        Use *use = fit->first->useHead;
                        while (use != nullptr)
                        {
                            if (use->user == (*it).get())
                            {
                                use->rmUse();
                            }
                            use = use->next;
                        }   
                        fit = Pi->from.erase(fit);
                    }
                    else{
                        fit++;
                    }
                }
                if(Pi->from.size()==1){
                    replaceVarByVar(Pi->reg, Pi->from[0].first);
                    deleteUser((*it).get());
                    it = bb->instructions.erase(it);
                }
                else{
                    it++;
                }
            }
            else{
                break;
            }
        }
    }
    return change;
}


void deleteOneFromOneToBB(FunctionPtr func){
    for(auto it = func->basicBlocks.begin();it!=func->basicBlocks.end();it++){
        auto bb = *it;
        //前缀为1
        if(bb->predBasicBlocks.size()==1){
            for(auto pred:bb->predBasicBlocks){
                if(auto bI = dynamic_cast<BrInstruction*>(pred->instructions.back().get())){
                    //前缀的后缀为1
                    if(!bI->exp){
                        assert(pred->succBasicBlocks.size()==1&&"has succBasicBlock not equal 1");
                        // auto br = bb->instructions.back();
                        pred->instructions.pop_back();
                        //bb不可能有phi
                        pred->instructions.insert(pred->instructions.end(),bb->instructions.begin(),bb->instructions.end());
                        pred->succBasicBlocks = bb->succBasicBlocks;
                        for(auto succ:bb->succBasicBlocks){
                            succ->predBasicBlocks.erase(bb);
                            succ->predBasicBlocks.insert(pred);
                            for(auto I:succ->instructions){
                                if(I->type == Phi){
                                    auto Pi = dynamic_cast<PhiInstruction*> (I.get());
                                    for(int i = 0;i<Pi->from.size();i++){
                                        if(Pi->from[i].second == bb){
                                            Pi->from[i].second = pred;
                                        }
                                    }
                                }
                                else{
                                    break;
                                }
                            }
                        }
                        it = func->basicBlocks.erase(it);
                        it--;
                    }
                }
            }
        }
    }
}

//to-do 多个return的merge先放着，感觉没啥用


void findFunctionBackedges(FunctionPtr func, vector<pair<BasicBlockPtr,BasicBlockPtr>>&Result){
    BasicBlockPtr BB = func->basicBlocks[0];
    if(BB->succBasicBlocks.empty()){
        return;
    }
    unordered_set<BasicBlockPtr> visited;
    vector<pair<BasicBlockPtr,unordered_set<BasicBlockPtr>::const_iterator>> visitStack;
    unordered_set<BasicBlockPtr> inStack;
    visited.insert(BB);
    visitStack.push_back({BB,BB->succBasicBlocks.begin()});
    inStack.insert(BB);
    do{
        pair<BasicBlockPtr, unordered_set<BasicBlockPtr>::const_iterator>&top = visitStack.back();
        BasicBlockPtr ParentBB = top.first;
        unordered_set<BasicBlockPtr>::const_iterator &I = top.second;
        bool FoundNew = false;
        while(I!=ParentBB->succBasicBlocks.end()){
            BB = *I++;
            if(visited.insert(BB).second){
                FoundNew = true;
                break;
            }

            if(inStack.count(BB)){
                Result.push_back({ParentBB,BB});
            }
        }

        if(FoundNew){
            inStack.insert(BB);
            visitStack.push_back({BB,BB->succBasicBlocks.begin()});
        }
        else{
            inStack.erase(visitStack.back().first);
            visitStack.pop_back();
        }
    }while(!visitStack.empty());

}

static bool simplifyCFGOnce(BasicBlockPtr bb, vector<BasicBlockPtr> loopHeaders){
    bool changed = false;
    assert(bb && bb->belongfunc && "block not embeded in function");
    assert(bb->endInstruction && "bb not end");
    
    return changed;
}


static bool iterativeSimplifyCFG(FunctionPtr func){
    bool LocalChange = true;
    bool Changed = false;
    vector<pair<BasicBlockPtr,BasicBlockPtr>> Edges;
    findFunctionBackedges(func,Edges);
    unordered_set<BasicBlockPtr> uniqueLoopHeaders;
    for(auto &Edge:Edges){
        uniqueLoopHeaders.insert(Edge.second);
    }
    vector<BasicBlockPtr> LoopHeaders(uniqueLoopHeaders.begin(),uniqueLoopHeaders.end());
    unsigned iterCnt = 0;
    while(LocalChange){
        assert(iterCnt++<100&&"Iterative simplification didn't converge");
        LocalChange = false;
        for(auto bbit = func->basicBlocks.begin();bbit!=func->basicBlocks.end();){
            BasicBlockPtr bb = *bbit++;
            if(simplifyCFGOnce(bb,LoopHeaders)){
                LocalChange = true;
            }
        }
        Changed |= LocalChange;

    }
    return Changed;
}

void simplifyCFG(FunctionPtr func){
    removeUnreachableBlocks(func);
    deleteOneFromOneToBB(func);
}