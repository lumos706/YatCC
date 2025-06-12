#include "inliner.h"

// #define INLINERBUg

#ifdef INLINERBUg
#define error(...) cerr<<__VA_ARGS__
#else
#define error(...)
#endif


//caller 调用该函数的函数  callee 该函数调用的函数
void findFunctionCallerAndCallee(FunctionPtr func){
    for(auto bb : func->basicBlocks){
        //在这里设置一下，前面好像有bug
        bb->belongfunc = func;
        for(auto I:bb->instructions){
            if(I->type == Call){
                CallInstruction* CI = dynamic_cast<CallInstruction*>(I.get());
                //func中调用了CI指向的func
                if(func->callee.find(CI->func)==func->callee.end()){
                    func->callee[CI->func] = 1;
                }
                else{
                    func->callee[CI->func]+=1;
                }
                //func是ci指向的func的调用者
                if(CI->func->caller.find(func)==CI->func->caller.end()){
                    CI->func->caller[func] = 1;
                }
                else{
                    CI->func->caller[func] += 1;
                }
                //调用该函数的指令

                if(!CI->func->callerIns.insert(I).second){
                    assert(false&&"findFunctionCaller error");
                }
            }
        }
    }
}




//copy个Value
static VariablePtr copyVariable(VariablePtr old){
    //arr的inner就不要了，因为在局部变量中其会转换成gep和store指令，所以可以在后续获得
    //intval floatval也都不要了，其以store、load指令存在，variable其实不应该有这几个量存在
    error("copyVariavle\n");
    VariablePtr ret = Variable::copy(old);
    error("copyVariavle1\n");
    //清除use，这些use会在之后copy指令的过程中重新建立
    ret->name = ret->name+".copy";
    ret->useHead = nullptr;
    ret->numUses = 0;
    return ret;
}


static ValuePtr getNewOperand(ValuePtr old, unordered_map<ValuePtr, ValuePtr>&ValueMap){
    error("entre getNewOperand\n");
    //const use same pointer
    if(old->isConst){
        error("const\n");
        return old;
    }
    error("getNewOperand1\n");
    if(ValueMap.find(old) == ValueMap.end()){
        //打个补丁，phi中可能会用到
        if(!dynamic_pointer_cast<Variable>(old)){
            ValueMap[old] =  ValuePtr(new Reg(old->type,old->name));
            return ValueMap[old];
        }
        ValueMap[old] = copyVariable(dynamic_pointer_cast<Variable>(old));
    }
    error("getNewOperand2\n");
    return ValueMap[old];
}



static InstructionPtr copyInstruction(InstructionPtr old, unordered_map<ValuePtr,ValuePtr>&ValueMap,
                                        unordered_map<BasicBlockPtr,BasicBlockPtr>&BBMap,
                                        unordered_map<LabelPtr,LabelPtr>&LabelMap){
    
    error("enter CopyInstruction\n");
    if(old->type == Return){
        ReturnInstruction* RI = dynamic_cast<ReturnInstruction*>(old.get());

        auto ret =  InstructionPtr(new ReturnInstruction(getNewOperand(RI->retValue,ValueMap),nullptr));
        //供后续阶段使用
        ValueMap[old->reg] = ret->reg;
        return ret;
    }
    else if(old->type == Br){
        BrInstruction* RI = dynamic_cast<BrInstruction*>(old.get());
        if(RI->exp){
            //这里的label使用label获得，其实block里有labelbbmap，但实在是太麻烦了，就算了。
            auto ret =  InstructionPtr(new BrInstruction(getNewOperand(RI->exp,ValueMap),LabelMap[RI->label_true],LabelMap[RI->label_false],nullptr));
            //供后续阶段使用
            ValueMap[old->reg] = ret->reg;
            return ret;
        }
        else{
            auto ret =  InstructionPtr(new BrInstruction(LabelMap[RI->label_true],nullptr));
            //供后续阶段使用
            ValueMap[old->reg] = ret->reg;
            return ret;
        }
    }
    else if(old->type == Alloca){
        AllocaInstruction* RI = dynamic_cast<AllocaInstruction*>(old.get());

        auto ret =  InstructionPtr(new AllocaInstruction(getNewOperand(RI->des,ValueMap),nullptr));
        //供后续阶段使用
        ValueMap[old->reg] = ret->reg;
        return ret;
    }
    else if(old->type == Store){
        StoreInstruction* RI = dynamic_cast<StoreInstruction*>(old.get());
        if(RI->gep){
            GetElementPtrInstruction* newGEP = dynamic_cast<GetElementPtrInstruction*>((ValueMap[RI->gep->reg])->I);
            auto ret =  InstructionPtr(new StoreInstruction(dynamic_pointer_cast<GetElementPtrInstruction>(newGEP->getSharedThis()), getNewOperand(RI->value,ValueMap),nullptr));
            //供后续阶段使用
            ValueMap[old->reg] = ret->reg;
            return ret;  
        }
        else{
            auto ret =  InstructionPtr(new StoreInstruction(getNewOperand(RI->des,ValueMap),getNewOperand(RI->value,ValueMap),nullptr));
            //供后续阶段使用
            ValueMap[old->reg] = ret->reg;
            return ret;   
        }
    }
    else if(old->type == Load){
        LoadInstruction* RI = dynamic_cast<LoadInstruction*>(old.get());
        if(RI->from->type->isPtr()){
            auto ret =  InstructionPtr(new LoadInstruction(getNewOperand(RI->from, ValueMap), getNewOperand(RI->to,ValueMap),nullptr));
            //供后续阶段使用
            ValueMap[old->reg] = ret->reg;
            return ret;  
        }
        else{
            auto ret =  InstructionPtr(new LoadInstruction(getNewOperand(RI->from,ValueMap),getNewOperand(RI->to,ValueMap),nullptr));
            //供后续阶段使用
            ValueMap[old->reg] = ret->reg;
            return ret;   
        }
    }
    else if(old->type == Call){
        CallInstruction* RI = dynamic_cast<CallInstruction*>(old.get());
        vector<ValuePtr> newArgv;
        for(int i = 0;i<RI->argv.size();i++){
            newArgv.push_back(getNewOperand(RI->argv[i],ValueMap));
        }
        auto ret =  InstructionPtr(new CallInstruction(RI->func, newArgv, nullptr));
        //供后续阶段使用
        ValueMap[old->reg] = ret->reg;
        return ret;
    }
    else if(old->type == Bitcast){
        BitCastInstruction* RI = dynamic_cast<BitCastInstruction*>(old.get());

        auto ret =  InstructionPtr(new BitCastInstruction(getNewOperand(RI->from, ValueMap), getNewOperand(RI->reg, ValueMap),nullptr, RI->toType));
        //供后续阶段使用
        ValueMap[old->reg] = ret->reg;
        return ret;
    }
    else if(old->type == Ext){
        ExtInstruction* RI = dynamic_cast<ExtInstruction*>(old.get());

        auto ret =  InstructionPtr(new ExtInstruction(getNewOperand(RI->from, ValueMap), RI->to,RI->isign,nullptr));
        //供后续阶段使用
        ValueMap[old->reg] = ret->reg;
        return ret;
    }
    else if(old->type == Sitofp){
        SitofpInstruction* RI = dynamic_cast<SitofpInstruction*>(old.get());

        auto ret =  InstructionPtr(new SitofpInstruction(getNewOperand(RI->from, ValueMap), nullptr));
        //供后续阶段使用
        ValueMap[old->reg] = ret->reg;
        return ret;
    }
    else if(old->type == Fptosi){
        FptosiInstruction* RI = dynamic_cast<FptosiInstruction*>(old.get());

        auto ret =  InstructionPtr(new FptosiInstruction(getNewOperand(RI->from, ValueMap), nullptr));
        //供后续阶段使用
        ValueMap[old->reg] = ret->reg;
        return ret;
    }
    else if(old->type == GEP){
        GetElementPtrInstruction* RI = dynamic_cast<GetElementPtrInstruction*>(old.get());
        vector<ValuePtr> newIndex;
        for(auto oldInd:RI->index){
            newIndex.push_back(getNewOperand(oldInd,ValueMap));
        }
        auto newFrom = getNewOperand(RI->from,ValueMap);
       
        auto ret =  InstructionPtr(new GetElementPtrInstruction(getNewOperand(RI->from,ValueMap),newIndex,nullptr));
         if(RI->from->type->isPtr()){
            ret->reg->type = dynamic_cast<PtrType*>(RI->from->type.get())->inner;
        }
        //供后续阶段使用
        ValueMap[old->reg] = ret->reg;
        return ret;
    }
    else if(old->type == Binary){
        error("binary \n");
        BinaryInstruction* RI = dynamic_cast<BinaryInstruction*>(old.get());


        auto ret =  InstructionPtr(new BinaryInstruction(getNewOperand(RI->a, ValueMap),getNewOperand(RI->b, ValueMap),RI->op, BasicBlockPtr(nullptr)));
        //供后续阶段使用
        error("binary2 \n");
        ValueMap[old->reg] = ret->reg;
        error("binary3 \n");
        return ret;
    }
    else if(old->type == Fneg){
        FnegInstruction* RI = dynamic_cast<FnegInstruction*>(old.get());

        auto ret =  InstructionPtr(new FnegInstruction(getNewOperand(RI->a, ValueMap), nullptr));
        //供后续阶段使用
        ValueMap[old->reg] = ret->reg;
        return ret;
    }
    else if(old->type == Icmp){
        IcmpInstruction* RI = dynamic_cast<IcmpInstruction*>(old.get());

        auto ret =  InstructionPtr(new IcmpInstruction(nullptr, getNewOperand(RI->a, ValueMap), getNewOperand(RI->b, ValueMap),RI->op));
        //供后续阶段使用
        ValueMap[old->reg] = ret->reg;
        return ret;
    }
    else if(old->type == Fcmp){
        FcmpInstruction* RI = dynamic_cast<FcmpInstruction*>(old.get());

        auto ret =  InstructionPtr(new FcmpInstruction(nullptr, getNewOperand(RI->a, ValueMap), getNewOperand(RI->b, ValueMap),RI->op));
        //供后续阶段使用
        ValueMap[old->reg] = ret->reg;
        return ret;
    }
    else if(old->type == Phi){
        error("Phi\n");
        PhiInstruction* RI = dynamic_cast<PhiInstruction*>(old.get());

        auto ret =  InstructionPtr(new PhiInstruction(nullptr, getNewOperand(RI->val, ValueMap)));
        //第二项参数实际上不需要了，后面直接能找到对应项，第二项参数留着会导致一个bug，即如果该phi是由后面用于替代return产生的，那么这个val就是call
        //而call不是变量，copy时出现错误
        // auto ret =  InstructionPtr(new PhiInstruction(nullptr, RI->val));
        error("Phi4\n");
        //供后续阶段使用
        ValueMap[old->reg] = ret->reg;
        return ret;
    }
    else{
        assert(false&&"unknown Inst type");
    }
}

//按照dfs的顺序遍历bb，来拷贝指令
//其实应该是拓扑排序，但因为有环，所以没办法
//dfs的是原BB，因为需要原BB的指令信息，且没有copyBB到原BB的映射，所以直接遍历原BB，反正结构是一样的
static void dfsBasicBlock(BasicBlockPtr root, unordered_map<BasicBlockPtr, bool>&vis, 
                            unordered_map<ValuePtr, ValuePtr>&ValueMap, 
                            unordered_map<BasicBlockPtr,BasicBlockPtr> BBMap,
                            unordered_map<LabelPtr,LabelPtr> LabelMap,
                            int callnum){
    BasicBlockPtr copyBasicBlock = BBMap[root];

    for(auto I:root->instructions){
        error("before copyIns\n");
        // I->print();
        auto copyIns = copyInstruction(I, ValueMap, BBMap,LabelMap);
        if(copyIns->reg) copyIns->reg->name=root->belongfunc->name+"."+copyIns->reg->name+".copy"+to_string(callnum);
        // copyIns->print();
        error("after copyIns\n");
        
        copyIns->basicblock = copyBasicBlock;
        copyBasicBlock->instructions.push_back(copyIns);
    }
    copyBasicBlock->setEndInstruction(copyBasicBlock->instructions.back());
    for(auto succ:root->succBasicBlocks){
        if(!vis.count(succ)){
            vis[succ] = true;
            dfsBasicBlock(succ, vis, ValueMap, BBMap, LabelMap,callnum);
        }
    }

}


//深拷贝一个函数，这样在替换完参数之后就可以直接把bb移过去
//alloca怎么处理呢
static FunctionPtr copyFunction(FunctionPtr func, Module& ir,int callNum){


    //这个func的名字就不需要随便加数字防止重名了，因为copy的函数最后会被销毁
    FunctionPtr newFunc = FunctionPtr(new Function(func->retVal->type,func->name+".copy",func->formArguments));

    //用于保存原func的value到copyFunc的映射，这样才能保持原本的结构，即一条指令用到了之前的结果，那么可以通过原func中的结果来找到新func的结果
    //而之前的结果一定在这之前存到了valuemap中
    unordered_map<ValuePtr,ValuePtr> ValueMap;
    unordered_map<BasicBlockPtr, BasicBlockPtr> BBMap;
    unordered_map<LabelPtr, LabelPtr> LabelMap;

    //copyFunc use the same globalVariable with oriFunc
    for(int i = 0;i<ir.globalVariables.size();i++){
        ValueMap[ir.globalVariables[i]] = ir.globalVariables[i];
    }
    //copy args
    vector<ValuePtr> newArgs;
    for(int i = 0;i<func->formArguments.size();i++){
        ValueMap[func->formArguments[i]] = copyVariable(dynamic_pointer_cast<Variable>(func->formArguments[i]));
        newArgs.push_back(ValueMap[func->formArguments[i]]);
    }

    newFunc->formArguments = newArgs;

    //copy BB
    vector<BasicBlockPtr> newBB;
    for(auto bb:func->basicBlocks){
        LabelPtr BBLabel = LabelPtr(new Label(func->name+bb->label->name+".copy"+to_string(callNum)));
        LabelMap[bb->label] = BBLabel;
        BBMap[bb] = BasicBlockPtr(new BasicBlock(BBLabel));
        newBB.push_back(BBMap[bb]);
    }
    for(auto  bb:func->basicBlocks){
        //完善cfg,保持原本的就够
        BasicBlockPtr copyBasicBlock = BBMap[bb];
        for(auto pred:bb->predBasicBlocks){
            copyBasicBlock->addPredBasicBlock(BBMap[pred]);
        }   
        for(auto succ:bb->succBasicBlocks){
            copyBasicBlock->addSuccBasicBlock(BBMap[succ]);
        }   
    }
    newFunc->basicBlocks = newBB;


    unordered_map<BasicBlockPtr, bool> vis;
    dfsBasicBlock(func->basicBlocks[0],vis,ValueMap,BBMap,LabelMap, callNum);

    //不能在dfs的过程中做，可能from还没遍历到，这就是不用拓扑序的弊端，但用不了(
    error("copyFunction5\n");
    for(auto bb:func->basicBlocks){
        for(auto ii:bb->instructions){
            if(ii->type == Phi){
                error("enter addFrom\n");
                PhiInstruction* SI = dynamic_cast<PhiInstruction*> (ii.get());
                error("enter addFrom1\n");
                // SI->print();
                error(ValueMap[SI->reg]->name);
                PhiInstruction* TI = dynamic_cast<PhiInstruction*> (ValueMap[SI->reg]->I);
                error("enter addFrom2\n");
                
                for(auto f:SI->from){
                    TI->addFrom(getNewOperand(f.first,ValueMap), BBMap[f.second]);
                }
                error("add finish\n");
            }
        }
    }
    error("copyFunction6\n");

    return newFunc;

}



void inlineFunction(CallInstruction * I,Module& ir,int callNum){
    BasicBlockPtr CallInBB = I->basicblock;
    FunctionPtr caller = CallInBB->belongfunc;
    FunctionPtr callee = I->func;
    

    //function中有许多基本块，将call指令前后的指令放在不同基本块，中间插入func的基本块，再将参数替换为传入的变量
    //需要处理跳转指令，基本块的关系，以及一些指令的布局，如alloca指令需放在entry块中，还需要避免重名

    error("inlineFunction1\n");
    auto it = CallInBB->instructions.begin();
    vector<InstructionPtr> newInst;
    for(;it!=CallInBB->instructions.end();it++){
        if((*it).get()==I){
            break;
        }
        newInst.push_back(*it);
    }
    //跳过Call
    it++;
    error("inlineFunction2\n");

    
    //后面随便加个数字防止重名
    LabelPtr BBAfterCallLabel = LabelPtr(new Label(callee->name+".ret"+to_string(callNum)));
    BasicBlockPtr BBAfterCall = BasicBlockPtr(new BasicBlock(BBAfterCallLabel));
    BBAfterCall->setBelongFunc(caller);

    error("inlineFunction3\n");
    vector<InstructionPtr> InstAfterCall;
    for(;it!=CallInBB->instructions.end();it++){
        InstAfterCall.push_back(*it);
        (*it)->basicblock = BBAfterCall;
    }
    error("inlineFunction4\n");
    CallInBB->instructions=newInst;

    BBAfterCall->succBasicBlocks = CallInBB->succBasicBlocks;
    for(auto succ:CallInBB->succBasicBlocks){
        // succ->predBasicBlocks.erase(CallInBB);
        // succ->predBasicBlocks[BBAfterCall] = true;
        succ->deletePredBasicBlock(CallInBB);
        succ->addPredBasicBlock(BBAfterCall);
        for(auto ii:succ->instructions){
            if(ii->type == Phi){
                for(auto&f:dynamic_cast<PhiInstruction*>(ii.get())->from){
                    if(f.second == CallInBB){
                        f.second = BBAfterCall;
                    }
                }
            }
        }
    }
    
    CallInBB->succBasicBlocks.clear();

    BBAfterCall->instructions = InstAfterCall;
    //保持原有的end就好
    BBAfterCall->setEndInstruction(InstAfterCall.back());

    FunctionPtr copyFunc = copyFunction(callee, ir, callNum);
    // copyFunc->print();
    
    CallInBB->instructions.push_back(InstructionPtr(new BrInstruction(copyFunc->basicBlocks[0]->label,CallInBB)));
    //维护这个属性，虽然好像没什么必要
    CallInBB->endInstruction = nullptr;
    CallInBB->setEndInstruction(CallInBB->instructions.back());
    CallInBB->addSuccBasicBlock(copyFunc->basicBlocks[0]);
    copyFunc->basicBlocks[0]->addPredBasicBlock(CallInBB);

    unordered_map<BasicBlockPtr, bool> vis;

    vector<InstructionPtr> calleeRetIns;

    error("inlineFunction5\n");
    // copyFunc->print();
    
    for(int i= 0,e = copyFunc->basicBlocks.size();i!=e;i++){
        error("inlineFunction51\n");
        copyFunc->basicBlocks[i]->belongfunc = caller;
        error("inlineFunction52\n");
        //返回基本块
        if(copyFunc->basicBlocks[i]->succBasicBlocks.size()==0){
            error("inlineFunction53\n");
            auto ret = copyFunc->basicBlocks[i]->instructions.back();
            error("inlineFunction54\n");
            if(ret->type == Return){

                calleeRetIns.push_back(ret);

                copyFunc->basicBlocks[i]->instructions.pop_back();
                InstructionPtr newBr = InstructionPtr(new BrInstruction(BBAfterCall->label, copyFunc->basicBlocks[i]));
                copyFunc->basicBlocks[i]->instructions.push_back(newBr);
                //维护endinstruction
                copyFunc->basicBlocks[i]->endInstruction = newBr;
            }
            else{
                assert(false&&"it should be return");
            }
        }
    }

    error("inlineFunction6\n");

    for(int i =0;i<calleeRetIns.size();i++){
        auto bb  = calleeRetIns[i]->basicblock;
        // bb->succBasicBlocks[BBAfterCall] = true;
        // BBAfterCall->predBasicBlocks[bb] = true;
        bb->addSuccBasicBlock(BBAfterCall);
        BBAfterCall->addPredBasicBlock(bb);
    }
    if(!callee->retVal->type->isVoid()){
        if(BBAfterCall->predBasicBlocks.size()==1){
            replaceVarByVar(I->reg,dynamic_cast<ReturnInstruction*>(calleeRetIns[0].get())->retValue);
        }
        else{
            InstructionPtr newPhi = InstructionPtr(new PhiInstruction(BBAfterCall, I->reg));
            BBAfterCall->instructions.insert(BBAfterCall->instructions.begin(),newPhi);
            replaceVarByVar(I->reg,newPhi->reg);
            for(int i =0;i<calleeRetIns.size();i++){
                auto PI = dynamic_cast<PhiInstruction*>(newPhi.get());
                PI->addFrom(dynamic_cast<ReturnInstruction*>(calleeRetIns[i].get())->retValue,calleeRetIns[i]->basicblock);
            }
        }

    }

    error("inlineFunction7\n");

    for(int i =0;i<copyFunc->formArguments.size();i++){
        error("inlineFunction71\n");
        ValuePtr nowArg = I->argv[i];
        error("inlineFunction72\n");
        ValuePtr formArg = copyFunc->formArguments[i];
        error("inlineFunction73\n");
        error(formArg->name<<endl);
        // formArg->useHead->user->print();
        replaceVarByVar(formArg, nowArg);
        error("inlineFunction74\n");
    }


    error("inlineFunction8\n");

    vector<InstructionPtr> allocaIns;
    vector<InstructionPtr> InsEraseAlloca;
    for(auto Ins : copyFunc->basicBlocks[0]->instructions){
        if(Ins->type==Alloca){
            allocaIns.push_back(Ins);
        }
        else{
            InsEraseAlloca.push_back(Ins);
        }
    }

    error("inlineFunction9\n");

    copyFunc->basicBlocks[0]->instructions = InsEraseAlloca;

    allocaIns.insert(allocaIns.end(),caller->basicBlocks[0]->instructions.begin(),caller->basicBlocks[0]->instructions.end());
    caller->basicBlocks[0]->instructions = allocaIns;
    vector<BasicBlockPtr> newBBList;
    int i = 0;
    for(;i<caller->basicBlocks.size();i++){
        if(caller->basicBlocks[i]!=CallInBB){
            newBBList.push_back(caller->basicBlocks[i]);
        }
        else{
            break;
        }
    }
    newBBList.push_back(caller->basicBlocks[i]);
    i++;
    newBBList.insert(newBBList.end(), copyFunc->basicBlocks.begin(),copyFunc->basicBlocks.end());
    newBBList.push_back(BBAfterCall);
    for(;i<caller->basicBlocks.size();i++){
        newBBList.push_back(caller->basicBlocks[i]);
    }
    caller->basicBlocks = newBBList;
}


void inliner(Module& ir){

    error("enter Inliner\n");

    //构建调用图
    for(auto func:ir.globalFunctions){
        if(!func->isLib){
            findFunctionCallerAndCallee(func);
        }
    }


    error("CG finish\n");

    queue<FunctionPtr> inlinerFunc;
    unordered_set<FunctionPtr> visited;
    //仅内联不调用函数的函数
    for(auto func:ir.globalFunctions){
        if(!func->isLib&&func->name!="main"&&func->callee.size()==0){
            inlinerFunc.push(func);
            if(!visited.insert(func).second){
                assert(false&&"inline function inlined");
            }
        }
    }

    error("find function can be inline\n");

    while(!inlinerFunc.empty()){
        FunctionPtr nowFunc = inlinerFunc.front();
        inlinerFunc.pop();
        int callNum = 0;
        error("before inline\n");
        for(auto I:nowFunc->callerIns){
            CallInstruction* CI = dynamic_cast<CallInstruction*>(I.get());
            inlineFunction(CI, ir, callNum++);
        }

        error("after Inline\n");
        
        for(auto C:(nowFunc->caller)){
            auto Caller = C.first;
            //删除调用者对其调用，因为全部都会inline，所以无所谓次数了
            Caller->callee.erase(nowFunc);
        }

        //该函数应该可以删除了，毕竟已经全部inline了
        for(auto it = ir.globalFunctions.begin();it!=ir.globalFunctions.end();it++){
            if((*it) == nowFunc){
                ir.globalFunctions.erase(it);
                break;
            }
        }

        //可能会产生新的没调用函数的函数
        for(auto func:ir.globalFunctions){
            if(!func->isLib&&func->name!="main"&&func->callee.size()==0&&!visited.count(func)){
                inlinerFunc.push(func);
                if(!visited.insert(func).second){
                    assert(false&&"visited insert fail");
                }
            }
        }
    }
}