#include "mem2reg.h"


//block和root为当前节点，dfsNum和rootNum是按照前序遍历的顺序的节点编号，used是标志每个节点是否被访问过，且记录每个bb的dfs下标
//block组织的是图G，root组织的是dfs树
void dfsTree(BasicBlockPtr bb,vector<BasicBlockPtr>&dfsNum, vector<node*>&rootNum,unordered_map<BasicBlockPtr, int>& used, node* root){
    dfsNum.push_back(bb);
    rootNum.push_back(root);
    used[bb] =dfsNum.size()-1;
    for(auto& succ:(bb->succBasicBlocks)){
        if(used.find(succ)==used.end()){
            root->son.push_back(new node(succ));
            root->son.back()->father = root;
            dfsTree(succ, dfsNum, rootNum, used,root->son.back());
        }
    }
}

int mEval(int id, vector<node *>& root,vector<BasicBlockPtr>& dfsNum, vector<int >& semi, unordered_map<BasicBlockPtr , int>& used){
    if(root[id]->bb == dfsNum[id]){
        return id;
    }
    else{
        queue<node*> list;
        int ret = id;
        for(int i=0;i<root[id]->son.size();i++){
            list.push(root[id]->son[i]);
        }
        while(!list.empty()){
            node* t = list.front();
            list.pop();
            if(semi[used[t->bb]]<semi[ret]){
                ret = used[t->bb];
            }
            for(int i=0;i<t->son.size();i++){
                list.push(t->son[i]);
            }
        }
        return ret;
    }
}

// DFS计算能支配一个基本块的所有基本块
static unordered_set<BasicBlockPtr> doms;
void computeAllDoms(BasicBlockPtr bb) {
    doms.insert(bb);
    bb->setAllDoms(doms);
    for(auto next : bb->getDominatorSon())
        computeAllDoms(next);
    doms.erase(bb);
}

void domTree(FunctionPtr func){
    doms.clear();
    //有bug
    //目前是正确的，但时间复杂度高，感觉像n^4
    for(auto bb:func->basicBlocks){
        // bb->directDominator = nullptr;
        bb->setDirectDominator(nullptr);
        bb->dominatorSon.clear();
        bb->DF.clear();
    }


    vector<BasicBlockPtr> dfsNum;
    vector<node*> rootNum;
    unordered_map<BasicBlockPtr , int> used;
    node* dfsRoot = new node(func->basicBlocks[0]);
    dfsTree(func->basicBlocks[0], dfsNum, rootNum, used, dfsRoot);


    vector<unordered_map<BasicBlockPtr, bool>> Dom(dfsNum.size());
    //先序遍历id为0的节点即entryBB,该bb的支配节点只会是他本身
    Dom[0][dfsNum[0]] = true;

    for(int i=1;i<dfsNum.size();i++){
        for(int j=0;j<dfsNum.size();j++){
            Dom[i][dfsNum[j]] = true;
        }
    }
    bool change = true;
    while(change){
        change = false;
        for(int i=0;i<dfsNum.size();i++){
            unordered_map<BasicBlockPtr, bool> newDom;
            for(int j=0;j<dfsNum.size();j++){
                bool flag = false;
                for(auto& pred:(dfsNum[i]->predBasicBlocks)){
                    flag = true;
                    if(Dom[used[pred]].find(dfsNum[j])==Dom[used[pred]].end()){
                        flag = false;
                        break;
                    }
                }
                if(flag){
                    newDom[dfsNum[j]] = true;
                }
            }
            newDom[dfsNum[i]] = true;
            if(Dom[i]!=newDom){
                Dom[i] = newDom;
                change = true;
            }
        }
    }

    // for(int i=0;i<dfsNum.size();i++){
    //     cerr<<"label:               "<<dfsNum[i]->label->name<<endl;
    //     for(auto& d:Dom[i]){
    //         cerr<<d.first->label->name<<endl;
    //     }
    // }


    //对每个BB BFS找到直接支配节点
    for(int i=1 ;i<dfsNum.size();i++){
        queue<BasicBlockPtr> list;
        unordered_map<BasicBlockPtr, bool> vis;
        vis[dfsNum[i]] = true;
        for(auto&pred:(dfsNum[i]->predBasicBlocks)){
            list.push(pred);
            vis[pred] = true;
        }
        while(!list.empty()){
            auto t = list.front();
            list.pop();
            if(Dom[i].find(t)!=Dom[i].end()){
                // dfsNum[i]->directDominator = t;
                dfsNum[i]->setDirectDominator(t);
                // t->dominatorSon[dfsNum[i]] = true;
                t->addDominatorSon(dfsNum[i]);
                break;
            }
            for(auto&pred:(t->predBasicBlocks)){
                if(vis.find(pred)==vis.end()){
                    list.push(pred);
                    vis[pred] = true;
                }
            }

        }
    }

    //计算DF
    for(int i=0;i<dfsNum.size();i++){
        if(dfsNum[i]->predBasicBlocks.size()>=2){
            for(auto &pred:(dfsNum[i]->predBasicBlocks)){
                auto runner = pred;
                while(runner!=dfsNum[i]->directDominator){
                    // runner->DF[dfsNum[i]] = true;
                    runner->addDF(dfsNum[i]);
                    if(runner->directDominator){
                        runner = runner->directDominator;
                    }
                    else{
                        break;
                    }
                }
            }
        }
    }

    computeAllDoms(func->getEntryBlock());
}

bool isADominatorB(BasicBlockPtr A, BasicBlockPtr B, BasicBlockPtr entry){
    if(A == entry){
        return true;
    }
    while(B!=entry){
        if(B == A){
            return true;
        }
        B = B->directDominator;
    }
    return false;
}


struct valBB{
    ValuePtr val;
    BasicBlockPtr BB;
    valBB(ValuePtr iv,BasicBlockPtr iBB):val{iv},BB{iBB}{};
};


//删除没有前驱的块
void eraseNotPredBB(FunctionPtr func){
    vector<BasicBlockPtr> newBB;
    newBB.push_back(func->basicBlocks[0]);
    for(int i=1;i<func->basicBlocks.size();i++){
        if(func->basicBlocks[i]->predBasicBlocks.size()>0){
            newBB.push_back(func->basicBlocks[i]);
        }
        else{
            //to-do  递归删除
            for(auto &succ:(func->basicBlocks[i]->succBasicBlocks)){
                succ->predBasicBlocks.erase(func->basicBlocks[i]);
            }
        }
    }
    func->basicBlocks = newBB;
}

void mem2reg(FunctionPtr func){
    eraseNotPredBB(func);

    auto entry = func->basicBlocks[0];
    unordered_map<ValuePtr, vector<BasicBlockPtr>> defBB;
    unordered_map<ValuePtr, vector<BasicBlockPtr>> useBB;

    //去除对应alloca的entry
    //可以被promote的alloca是只被load和store使用的alloca，在该比赛中就是float和int，实际上ptr应该也不会出现，但是我忘了为啥这样写了，就留着吧
    vector<InstructionPtr> newEntryInst;
    for(int i=0;i<entry->instructions.size();i++){
        if(entry->instructions[i]->type==Alloca){
            auto tI = ((AllocaInstruction*)(entry->instructions[i].get()));
            if(tI->des->type->isFloat()||tI->des->type->isInt()||tI->des->type->isPtr()){
                //记录需要删除的alloca变量
                defBB[((AllocaInstruction*)(entry->instructions[i].get()))->des] = {};
                useBB[((AllocaInstruction*)(entry->instructions[i].get()))->des] = {};
            }
            else{
                newEntryInst.emplace_back(entry->instructions[i]);
            }
        }
        else{
            newEntryInst.emplace_back(entry->instructions[i]);
        }
    }
    //记录store的块
    for(auto &bb:(func->basicBlocks)){
        for(auto &inc:(bb->instructions)){
            if(inc->type == Store&&defBB.find(((StoreInstruction*)(inc.get()))->des)!=defBB.end()){
                defBB[((StoreInstruction*)(inc.get()))->des].emplace_back(bb);
            }
        }
    }
    //记录load的块
    for(auto &bb:(func->basicBlocks)){
        for(auto &inc:(bb->instructions)){
            if(inc->type == Load&&useBB.find(((LoadInstruction*)(inc.get()))->from)!=useBB.end()){
                useBB[((LoadInstruction*)(inc.get()))->from].emplace_back(bb);
            }
        }
    }
    unordered_map<BasicBlockPtr,ValuePtr> inworkList;

    //记录在哪些基本块中插入哪些变量的phi
    unordered_map<BasicBlockPtr,ValuePtr> inserted;
    for(auto&bb:func->basicBlocks){
        inworkList[bb] = nullptr;
        inserted[bb] = nullptr;
    }
    queue<BasicBlockPtr> workList;

    //A variable (virtual register) is live at some point in the program if it has
    // previously been defined by an instruction and will be used by an
    // instruction in the future. It is dead otherwise.


    //对于每个store过的变量
    for(auto &valDef:defBB){
        queue<BasicBlockPtr> liveInBBWorkList;
        unordered_map<BasicBlockPtr, bool> liveInBB;
        //从vector变成unordered_map提升效率    定义(Store)该变量的基本块
        unordered_map<BasicBlockPtr, bool> defMap;
        for(int i=0;i<valDef.second.size();i++){
            defMap[valDef.second[i]] = true; 
        }

        //对每个使用load过这个变量的bb
        for(auto &use:(useBB[valDef.first])){
            //如果同个基本块内没有store的话，则放入liveInBBWorkList里面
            if(defMap.find(use) != defMap.end()){
                bool flag = true;
                for(auto & I:(use->instructions)){
                    //如果先遇到load该变量的指令，则放入liveinbbworklist
                    //如果先遇到store该变量的指令，则不需要放入，因为之后用的就是该store的值了
                    //如果啥都没遇到，则不可能(
                    if(I->type == Store){
                        auto tempI = (StoreInstruction*)(I.get());
                        if(tempI->des!=valDef.first){
                            continue;
                        }
                        flag = false;
                        break;
                    }
                    else if(I->type == Load){
                        auto tempI = (LoadInstruction*)(I.get());
                        if(tempI->from!=valDef.first){
                            continue;
                        }
                        break;
                    }
                }
                if(flag){
                    liveInBBWorkList.push(use);
                }
            }
            else{
                liveInBBWorkList.push(use);
            }
        }
        //将有use和没有use的livebb找出来
        while(!liveInBBWorkList.empty()){
            auto tempBB = liveInBBWorkList.front();
            liveInBBWorkList.pop();
            if(liveInBB.find(tempBB)!=liveInBB.end()){
                continue;
            }
            liveInBB[tempBB] = true;
            for(auto& pred:(tempBB->predBasicBlocks)){
                //如果没有use但有def的，不是livebb
                if(defMap.find(pred)!=defMap.end()){
                    continue;
                }
                liveInBBWorkList.push(pred);
            }
        }


        for(auto &t:(valDef.second)){
            inworkList[t] = valDef.first;
            workList.push(t);
        }
        //unordered_map<BasicBlockPtr,bool> finish;
        while(!workList.empty()){
            auto now = workList.front();
            workList.pop();
            for(auto &df:now->DF){
                if(inserted[df] != valDef.first && liveInBB.find(df) != liveInBB.end()){

                    auto mBegin = df->instructions.begin();
                    auto phi = InstructionPtr(new PhiInstruction(df, valDef.first));
                    df->instructions.emplace(mBegin,phi);
                    inserted[df] = valDef.first;
                    if(inworkList[df]!= valDef.first){
                        inworkList[df] = valDef.first;
                        workList.push(df);
                    }
                }
            }
        }
    }




    entry->instructions = newEntryInst;
    //每个变量一个stack
    unordered_map<ValuePtr ,vector<valBB>> stak;
    for(auto & varDef: defBB){
        //默认初始化为0
        if(varDef.first->type->isInt())
            stak[varDef.first] = {valBB(Const::getConst(Type::getInt(), 0),entry)};
        else{
            stak[varDef.first] = {valBB(Const::getConst(Type::getFloat(), float(0)),entry)};
        }
    }

    stack<BasicBlockPtr> list; 
    vector<InstructionPtr> newInst;
    list.push(entry);


    unordered_map<BasicBlockPtr,bool> visited;
    visited[entry] = true;

    
    while(!list.empty()){
        auto bb = list.top();
        //cerr<<bb->label->name<<endl;
        list.pop();
        //先记录要留下的指令，再统一删除，防止超时
        newInst.clear();
        //cerr<<"label:    "<<bb->label->name<<endl;
        for(int i=0;i<bb->instructions.size();i++){
            if(bb->instructions[i]->type == Load){
                auto to = ((LoadInstruction*)(bb->instructions[i].get()))->to;
                auto from = ((LoadInstruction*)(bb->instructions[i].get()))->from;
                //cerr<<"load:   "<<to->name<<" "<<from->name<<endl;
                if(stak.find(from)!=stak.end()){
                    //cerr<<"mmmm:   "<<to->name<<" "<<from->name<<endl;
                    //cerr<<"load pre: "<< stak[from.get()].back().val->name<<endl;
                    // while(!isADominatorB(stak[from.get()].back().BB, bb, entry.get())){
                    //     stak[from.get()].pop_back();
                    //     //cerr<<"load pre: "<< stak[from.get()].back().val->name<<endl;
                    // }
                    int stakIdx = stak[from].size()-1;
                    while(!isADominatorB(stak[from][stakIdx].BB, bb, entry)){
                        stakIdx--;
                    }
                    //cerr<<"after pre: "<< stak[from.get()].back().val->name<<endl;
                    //replaceVarByVar(to,stak[from.get()].back().val);
                    replaceVarByVar(to,stak[from][stakIdx].val);
                    // done in replaceVarByVar
                    // if(!rmInstructionUse(bb->instructions[i], from)){
                    //     cerr<<"error\n";
                    // }
                    rmInstructionUse(bb->instructions[i], from);
                }
                else{
                    newInst.emplace_back(bb->instructions[i]);
                }
            }
            //定义
            else if(bb->instructions[i]->type == Store){
                auto des = ((StoreInstruction*)(bb->instructions[i].get()))->des;
                auto value = ((StoreInstruction*)(bb->instructions[i].get()))->value;
                //cerr<<"store:   "<<des->name<<" "<<value->name<<endl;

                if(stak.find(des)!=stak.end()){
                    //cerr<<"kkk:   "<<des->name<<endl;
                    // while(!isADominatorB(stak[des.get()].back().BB, bb, entry.get())){
                    //     stak[des.get()].pop_back();
                    // }
                    // int stakIdx = stak[des].size()-1;
                    // while(!isADominatorB(stak[des][stakIdx].BB, bb, entry)){
                    //     stakIdx--;
                    // }
                    stak[des].emplace_back(valBB(value,bb));
                    if(!rmInstructionUse(bb->instructions[i], des)){
                        assert(false&&"cannot rm Store InstructionUse");
                    }
                    if(!rmInstructionUse(bb->instructions[i], value)){
                        assert(false&&"cannot rm Store InstructionUse");
                    }
                    // if(value->name=="binary3"){
                    //         cerr<<"pll   "<<value->numUses<<endl;
                    // }
                }
                else{
                    newInst.emplace_back(bb->instructions[i]);
                }
            }
            //定义
            else if(bb->instructions[i]->type == Phi){
                auto val = ((PhiInstruction*)(bb->instructions[i].get()))->val;
                auto reg = ((PhiInstruction*)(bb->instructions[i].get()))->reg;
                //cerr<<"phi:   "<<reg->name<<" "<<val->name<<endl;
                if(stak.find(val)!=stak.end()){
                    //cerr<<"phi:   "<<reg->name<<" "<<val->name<<endl;
                    // int stakIdx = stak[val].size()-1;
                    // while(!isADominatorB(stak[val][stakIdx].BB, bb, entry)){
                    //     stakIdx--;
                    // }

                    // while(!isADominatorB(stak[val].back().BB, bb, entry)){
                    //     stak[val].pop_back();
                    // }

                    stak[val].emplace_back(valBB(reg,bb));
                }
                newInst.emplace_back(bb->instructions[i]);
                //phi指令不用删除
            }
            else{
                newInst.emplace_back(bb->instructions[i]);
            }
        }

        for(auto &succ:(bb->succBasicBlocks)){
            //cerr<<succ.first->label->name<<endl;
            for(auto ins:(succ->instructions)){
                if(ins->type == Phi){
                    auto tempIns = (PhiInstruction*)(ins.get());
                    auto val = tempIns->val;
                    auto reg = tempIns->reg;
                    
                    if(stak.find(val)!=stak.end()){
                        //while(!isADominatorB(stak[val].back().BB, succ.first, entry)){
                        //cerr<<reg->name<<val->name<<endl;
                        //cerr<<"pre:    "<<stak[val].back().val->name<<endl;
                        // while(!isADominatorB(stak[val].back().BB, bb, entry)){
                        //     stak[val].pop_back();
                        // }
                        int stakIdx = stak[val].size()-1;
                         while(!isADominatorB(stak[val][stakIdx].BB, bb, entry)){
                            stakIdx--;
                        }  
                        //cerr<<"after:    "<<stak[val].back().val->name<<endl;
                        // auto newVal = stak[val].back().val;
                        auto newVal = stak[val][stakIdx].val;
                        tempIns->addFrom(newVal,bb);
                        // if(newVal->name=="binary3"){
                        //     cerr<<"oll0000   "<<newVal->numUses<<endl;
                        // }
                        // newUse(newVal.get(),ins.get());
                        // if(newVal->name=="binary3"){
                        //     cerr<<"oll   "<<newVal->numUses<<endl;
                        // }
                    }
                }
                //Phi一定在最前面
                else{break;}
            }
        }

        bb->instructions = newInst;
        // for(auto &domson:(bb->dominatorSon)){
        //     list.push(domson.first);
        // }
        for(auto &succ:(bb->succBasicBlocks)){
            if(visited.find(succ)==visited.end()){
                list.push(succ);
                visited[succ] = true;
            }
        }
    }

    // unordered_map<Value* ,ValuePtr> stak;
    // for(auto & varDef: defBB){
    //     //默认初始化为0
    //     stak[varDef.first] = ValuePtr(new Const(Type::getInt(), 0));
    // }

    // queue<BasicBlock*> list; 
    // vector<InstructionPtr> newInst;
    // list.push(entry.get());

    // unordered_map<BasicBlock*,bool> visited;
    // visited[entry.get()] = true;
    // while(!list.empty()){
    //     auto bb = list.front();
    //     list.pop();
    //     //先记录要留下的指令，再统一删除，防止超时
    //     newInst.clear();
    //     for(int i=0;i<bb->instructions.size();i++){
    //         if(bb->instructions[i]->type == Load){
    //             auto to = ((LoadInstruction*)(bb->instructions[i].get()))->to;
    //             auto from = ((LoadInstruction*)(bb->instructions[i].get()))->from;
    //             if(stak.find(from.get())!=stak.end()){
    //                 replaceVarByVar(to,stak[from.get()]);
    //             }
    //             else{
    //                 newInst.emplace_back(bb->instructions[i]);
    //             }
    //         }
    //         //定义
    //         else if(bb->instructions[i]->type == Store){
    //             auto des = ((StoreInstruction*)(bb->instructions[i].get()))->des;
    //             auto value = ((StoreInstruction*)(bb->instructions[i].get()))->value;
    //             if(stak.find(des.get())!=stak.end()){
    //                 stak[des.get()]=value;
    //             }
    //             else{
    //                 newInst.emplace_back(bb->instructions[i]);
    //             }
    //         }
    //         //定义
    //         else if(bb->instructions[i]->type == Phi){
    //             auto val = ((PhiInstruction*)(bb->instructions[i].get()))->val;
    //             auto reg = ((PhiInstruction*)(bb->instructions[i].get()))->reg;
    //             if(stak.find(val.get())!=stak.end()){
    //                 stak[val.get()]=reg;
    //             }
    //             newInst.emplace_back(bb->instructions[i]);
    //             //phi指令不用删除
    //         }
    //         else{
    //             newInst.emplace_back(bb->instructions[i]);
    //         }
    //     }

    //     for(auto &succ:(bb->succBasicBlocks)){
    //         for(auto & ins:(succ.first->instructions)){
    //             if(ins->type == Phi){
    //                 auto tempIns = (PhiInstruction*)(ins.get());
    //                 auto val = tempIns->val;
    //                 if(stak.find(val.get())!=stak.end()){
    //                     auto newVal = stak[val.get()];
    //                     tempIns->addFrom(ValuePtr(newVal),BasicBlockPtr(bb));
    //                     newUse(newVal.get(),ins.get());
    //                 }
    //             }
    //             //Phi一定在最前面
    //             else{break;}
    //         }
    //     }

    //     bb->instructions = newInst;
    //     for(auto &succ:(bb->succBasicBlocks)){
    //         if(visited.find(succ.first)==visited.end()){
    //             list.push(succ.first);
    //             visited[succ.first] = true;
    //         }
    //     }
    // }

}
