#include "loopUnroll.h"
#include "LICM.h"

int copy_num =0;

void unuseall(InstructionPtr ins)
{
    auto operands_num = ins->getNumOperands();
    for(int i=0;i<operands_num;i++)
    {
        auto operand = ins->getOperand(i);
        if(operand)
        {
            deleteUser(operand);
        }
    }
}


void error(string s)
{
    cerr << s << endl;
}

//copy个Value
static VariablePtr copyVariable(VariablePtr old){
    VariablePtr ret = Variable::copy(old);
    ret->name = ret->name+".copy";
    ret->useHead = nullptr;
    ret->numUses = 0;
    return ret;
}


static ValuePtr getNewOperand(ValuePtr old, unordered_map<ValuePtr, ValuePtr>&ValueMap){
    if(old->isConst){
        // error("const\n");
        return old;
    }
    // error("getNewOperand1\n");
    if(ValueMap.find(old) == ValueMap.end()){
        //打个补丁，phi中可能会用到
        if(!dynamic_pointer_cast<Variable>(old)){
            ValueMap[old] =  ValuePtr(new Reg(old->type,old->name+".unroll.copy"));
            return ValueMap[old];
        }
        ValueMap[old] = copyVariable(dynamic_pointer_cast<Variable>(old));
    }
    // error("getNewOperand2\n");

    return ValueMap[old];
}



static InstructionPtr copyInstruction(InstructionPtr old, unordered_map<ValuePtr,ValuePtr>&ValueMap,
                                        unordered_map<BasicBlockPtr,BasicBlockPtr>&BBMap,
                                        unordered_map<LabelPtr,LabelPtr>&LabelMap, FunctionPtr func){
    
    // error("enter CopyInstruction\n");
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
            // cerr << "Copyting Branch" << endl;
            if(LabelMap[RI->label_true]==nullptr)
            {
                // cerr << "Label 1 null "<< endl;
            }
            if(LabelMap[RI->label_false]==nullptr)
            {
                // cerr << "Label 2 null "<< endl;
            }
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
            auto nreg = func->getReg(RI->from->type);
            auto ret =  InstructionPtr(new LoadInstruction(getNewOperand(RI->from, ValueMap), getNewOperand(nreg,ValueMap),nullptr));
            // auto ret =  InstructionPtr(new LoadInstruction(getNewOperand(RI->from, ValueMap), getNewOperand(RI->to,ValueMap),nullptr));
            //供后续阶段使用
            ValueMap[old->reg] = ret->reg;
            return ret;  
        }
        else{
            auto nreg = func->getReg(RI->from->type);
            auto ret =  InstructionPtr(new LoadInstruction(getNewOperand(RI->from, ValueMap), getNewOperand(nreg,ValueMap),nullptr));
            // auto ret =  InstructionPtr(new LoadInstruction(getNewOperand(RI->from,ValueMap),getNewOperand(RI->to,ValueMap),nullptr));
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
        // error("binary \n");
        BinaryInstruction* RI = dynamic_cast<BinaryInstruction*>(old.get());


        auto ret =  InstructionPtr(new BinaryInstruction(getNewOperand(RI->a, ValueMap),getNewOperand(RI->b, ValueMap),RI->op, BasicBlockPtr(nullptr)));
        //供后续阶段使用
        // error("binary2 \n");
        ValueMap[old->reg] = ret->reg;
        // error("binary3 \n");
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
        // error("Phi\n");
        PhiInstruction* RI = dynamic_cast<PhiInstruction*>(old.get());

        auto ret =  InstructionPtr(new PhiInstruction(nullptr, getNewOperand(RI->val, ValueMap)));
        //第二项参数实际上不需要了，后面直接能找到对应项，第二项参数留着会导致一个bug，即如果该phi是由后面用于替代return产生的，那么这个val就是call
        //而call不是变量，copy时出现错误
        // auto ret =  InstructionPtr(new PhiInstruction(nullptr, RI->val));
        // error("Phi4\n");
        //供后续阶段使用
        ValueMap[old->reg] = ret->reg;
        return ret;
    }
    else{
        assert(false&&"unknown Inst type");
    }
}



LoopPtr getLoopCopy(LoopPtr loop, std::unordered_map<ValuePtr, ValuePtr>& LoopValueCpy, unordered_map<ValuePtr, ValuePtr>& CurrentIncomingValue, BasicBlockPtr exitblock, FunctionPtr func)
{
    // cerr << "Entering Loop copy" << endl;
    // auto headerCpy = BasicBlock::Create(loop->getHeader()->getName() + ".unrollcpy", loop->getParent());
    string now_num = to_string(copy_num++);
    LabelPtr BBLabel = LabelPtr(new Label(loop->getHeader()->label->name+".copy"+now_num));

    auto headerCpy = BasicBlockPtr(new BasicBlock(BBLabel));

    LabelPtr latchLabel = LabelPtr(new Label(loop->latchBlock->label->name+".copy"+now_num));

    auto latchCpy = (loop->getHeader() == loop->latchBlock) ? headerCpy : BasicBlockPtr(new BasicBlock(latchLabel));
    
    auto loopCpy = LoopPtr(new Loop(headerCpy,1));

    loopCpy->setLatchBlock(latchCpy);

    std::unordered_map<PhiInstruction*, PhiInstruction*> queuePhi;

    unordered_map<ValuePtr,ValuePtr> ValueMap;
    unordered_map<BasicBlockPtr, BasicBlockPtr> loopBBMap;
    unordered_map<LabelPtr, LabelPtr> LabelMap;
    unordered_map<InstructionPtr, InstructionPtr> insMap;

    for(auto bb : loop->getBasicBlocks())
        if(bb == loop->getHeader())
        {
            loopBBMap[bb] = headerCpy;
            LabelMap[bb->label] = headerCpy->label;
            // LoopValueCpy[bb] = headerCpy;
        }
        else if(bb == loop->getLatchBlock())
        {
            loopBBMap[bb] = latchCpy;
            LabelMap[bb->label] = latchCpy->label;
            // LoopValueCpy[bb] = latchCpy;
            loopCpy->addBasicBlock(latchCpy);
        }
        else if(bb != loop->getHeader() && bb != loop->getLatchBlock())
        {
            LabelPtr BBLabel = LabelPtr(new Label(bb->label->name+".unrollcpy"+now_num));
            auto loopBodyCpy = BasicBlockPtr(new BasicBlock(BBLabel));
            loopBBMap[bb] = loopBodyCpy;
            LabelMap[bb->label] = BBLabel;
            // LoopValueCpy[bb] = loopBodyCpy;
            loopCpy->addBasicBlock(loopBodyCpy);

        }

    LabelMap[exitblock->label] = exitblock->label; 

    // maybe wrong

    for(auto bb : loop->getBasicBlocks())
    {
        if(bb == loop->getHeader())
        {
            for(auto instr : bb->instructions)
            {
                if(instr->type == Phi)
                    continue;
                auto instrCpy = copyInstruction(instr, CurrentIncomingValue, loopBBMap, LabelMap, func);
                headerCpy->pushInstruction(instrCpy);
                instrCpy->basicblock = headerCpy;
            }
        }
        else if(bb == loop->getLatchBlock())
        {
            for(auto instr : bb->instructions)
            {
                auto instrCpy = copyInstruction(instr, CurrentIncomingValue, loopBBMap, LabelMap, func);
                latchCpy->pushInstruction(instrCpy);
                instrCpy->basicblock = latchCpy;
                if(instrCpy ->type ==Binary && instr -> type ==Binary)
                {
                    auto bi = dynamic_cast<BinaryInstruction*>(instr.get());
                    auto bi_cpy = dynamic_cast<BinaryInstruction*>(instrCpy.get());
                    auto v1 = bi->a;
                    auto v2 = bi->b;
                    if(v1->I && v1->I->type==Phi && loop == v1->I->basicblock->loop)
                    {
                        CurrentIncomingValue[v1] = bi_cpy->reg;
                    }
                    if(v2->I && v2->I->type==Phi && loop == v2->I->basicblock->loop)
                    {
                        CurrentIncomingValue[v2] = bi_cpy->reg;
                    }
                }
                if(instr->type==Phi)
                {
                    auto phi = dynamic_cast<PhiInstruction*>(instr.get());
                    if(instrCpy->type==Phi)
                    {
                        auto phiCpy = dynamic_cast<PhiInstruction*>(instrCpy.get());
                        queuePhi[phi] = phiCpy;
                    }
                        
                }
                    
            }
        }
        else
        {
            // cerr << "Loop stage 2 else" << endl;
            for(auto instr : bb->instructions)
            {
                auto instrCpy = copyInstruction(instr, CurrentIncomingValue, loopBBMap, LabelMap, func);
                // LoopValueCpy[instr] = instrCpy;
                auto loopBodyCpy = loopBBMap[bb];
                loopBodyCpy->pushInstruction(instrCpy);
                instrCpy->basicblock = loopBodyCpy;
                if(instrCpy ->type ==Binary && instr -> type ==Binary)
                {
                    auto bi = dynamic_cast<BinaryInstruction*>(instr.get());
                    auto bi_cpy = dynamic_cast<BinaryInstruction*>(instr.get());
                    auto v1 = bi->a;
                    auto v2 = bi->b;
                    // if(bi->op != '+' || bi->op != '-')
                    //     continue;
                    if(v1 ->I && v1->I->type==Phi)
                    {
                        CurrentIncomingValue[v1] = bi_cpy->reg;
                    }
                    if(v2 ->I && v2->I->type==Phi)
                    {
                        CurrentIncomingValue[v2] = bi_cpy->reg;
                    }
                }
                if(instr->type==Phi)
                {
                    auto phi = dynamic_cast<PhiInstruction*>(instr.get());
                    if(instrCpy->type ==Phi)
                    {
                        auto phiCpy = dynamic_cast<PhiInstruction*>(instrCpy.get());
                        queuePhi[phi] = phiCpy;
                    }
                        
                }
                    
            }
        }
    }

    // cerr << "Loop copy stage 2" << endl;
    // if(copy_num==2)
    // for(auto instr: latchCpy->instructions)
    // {
    //     cerr << instr->getName() << endl;
    // }

    for(auto iter : loopBBMap)
    {
        auto bb = iter.first;
        auto bbCpy = iter.second;
        // dumpBasickBlock(bbCpy);
        if(bb != loop->getHeader())
            for(auto pred : bb->predBasicBlocks)
                bbCpy->predBasicBlocks.insert(loopBBMap[pred]);
        if(bb != loop->getLatchBlock())
            for(auto succ : bb->succBasicBlocks)
                bbCpy->succBasicBlocks.insert(loopBBMap[succ]);
        // dumpBasickBlock(bbCpy);
    }

    // cerr << "Loop copy stage 3" << endl;

    for(auto iter : queuePhi)
    {
        auto phi = iter.first;
        auto phiCpy = iter.second;
        for(auto iter : phi->from)
        {
            auto val = iter.first;
            auto pred = iter.second;
            // phiCpy->replaceValue
            phiCpy->addFrom(val, loopBBMap[pred]);
        }
    }

    std::unordered_map<PhiInstruction*, ValuePtr> phiAssignLater;
    for(auto instr : loop->getHeader()->instructions)
    {
        if(instr->type==Phi)
        {
            auto phi = dynamic_cast<PhiInstruction*>(instr.get());
            for(auto iter : phi->from)
            {
                auto val = iter.first;
                auto pred = iter.second;
                if(pred == loop->getLatchBlock())
                    phiAssignLater[phi] = val;
            }
                
        }
        else
            break;
    }   
    return loopCpy;
}


void runOnLoop(LoopPtr loop, FunctionPtr func)
{
    int maxTripCount = 80;
    copy_num = 0;

    auto legalLoop = [&](LoopPtr loop) -> bool {
        // 1. Loop can not have break. At our simplify loop continue become 
        // to the common for loop
        if(loop->getExitingBlocks().size() != 1 || loop->getExitBlocks().size() != 1 || loop->getSubLoops().size())
        {
            return false;    
        } 
                                   // It must to be 1 at any time.
        auto trip = loop->getTripCount();
        if( !trip)
        {
            // std::cerr << "333" <<std::endl;
            return false;
        }
            
        else if(trip != 60 && trip != 300 && trip!= 20)
        {
            // std::cerr << "444" <<std::endl;
            return false;
        }
        // 3. Do not have any function call.
        for(auto bb : loop->getBasicBlocks())
            for(auto instr : bb->instructions)
                if(instr->type == Call)
                    return false;
        
        for(auto bb : loop->getBasicBlocks())
            for(auto instr : bb->instructions)
                if(instr->type == Icmp && bb != loop->header)
                    return false;
        return true;
    };


    if(!legalLoop(loop))
    {
        return;
    }
    else
    { }

        
    
    auto header = loop->getHeader();

    auto latch = loop->latchBlock;
    auto exit = *loop->getExitBlocks().begin();

    std::unordered_map<ValuePtr, ValuePtr> CurrentIncomingValue;
    std::vector<InstructionPtr> headerPhi;

    std::unordered_map<ValuePtr, ValuePtr> tmpmap;

    int itrtimes = 0;

    ValuePtr tracephi = nullptr;
    for(auto instr : header->instructions)
    {
        int phitimes = 0;
        if(instr->type==Phi)
        {
            auto phi  = (PhiInstruction*)(instr.get());
            headerPhi.push_back(instr);
            if(phi->reg->getStr()=="%phi2")
                tracephi = phi->reg;
            for(auto it : phi->from)
            {
                auto val = it.first;
                auto pred = it.second;
                if(pred == nullptr)
                    continue;
                if(pred == latch)
                {
                    CurrentIncomingValue[phi->reg] = val;
                }
            }
        }
        else
            break;
    }

    for(auto instr: latch->instructions)
    {
        if(instr->type ==Binary)
        {
            auto bi = dynamic_cast<BinaryInstruction*>(instr.get());
            auto reg = bi -> reg;
            if(CurrentIncomingValue.find(reg)==CurrentIncomingValue.end())
            {
                CurrentIncomingValue[reg] = reg;
            }
            if(bi->a && CurrentIncomingValue.find(bi->a)==CurrentIncomingValue.end() && !bi->a->isConst)
            {
                CurrentIncomingValue[bi->a] = bi->a;
            }
            if(bi->b && CurrentIncomingValue.find(bi->b)==CurrentIncomingValue.end() && !bi->b->isConst)
            {
                CurrentIncomingValue[bi->b] = bi->b;
            }
        }
        else if(instr->type == GEP)
        {
            auto gep = dynamic_cast<GetElementPtrInstruction*>(instr.get());
            auto reg = gep->reg;
            if(CurrentIncomingValue.find(reg) == CurrentIncomingValue.end())
            {
                CurrentIncomingValue[reg] = reg;
            }
            if(gep->from && CurrentIncomingValue.find(gep->from) ==CurrentIncomingValue.end())
            {
                CurrentIncomingValue[gep->from] = gep->from;
            }
        }
        else if(instr->type == Store)
        {
            auto str = dynamic_cast<StoreInstruction*>(instr.get());
            auto reg = str->des;
            if(CurrentIncomingValue.find(reg) == CurrentIncomingValue.end())
            {
                CurrentIncomingValue[reg] = reg;
            }
            if(str->value && CurrentIncomingValue.find(str->value) ==CurrentIncomingValue.end())
            {
                CurrentIncomingValue[str->value] = str->value;
            }
        }
    }

    for(auto iter: CurrentIncomingValue)
    {
        cout << "value->value is " << iter.first->name << " -> " << iter.second->name << endl;
    }

    auto itr = --latch->instructions.end();
    auto latchCondBr =  *itr;
    latch->instructions.erase(itr);
    latch->endInstruction = nullptr;
    unuseall(latchCondBr);
    latch->succBasicBlocks.clear();

    std::vector<BasicBlockPtr> latchHeaderEdge;
    BasicBlockPtr preLatch = latch;
    BasicBlockPtr nowHeader = nullptr;

    auto trip = loop->getTripCount();

    std::unordered_map<ValuePtr, ValuePtr> copyMap;
    latchHeaderEdge.push_back(preLatch);
    for(int i = 1; i < trip; i++)
    {
        auto loopCpy = getLoopCopy(loop, copyMap, CurrentIncomingValue,exit, func);

        auto headerCpy = loopCpy->getHeader();
        auto latchCpy = loopCpy->getLatchBlock();

        nowHeader = headerCpy;
        latchHeaderEdge.push_back(latchCpy);
        preLatch = latchCpy;
    }

    latchHeaderEdge.push_back(exit);

    for(auto instr : exit->instructions)
    {
        if(instr->type == Binary)
        {
            auto bi = dynamic_cast<BinaryInstruction*>(instr.get());
            if(bi->a&&bi->a->I && bi->a->I->type == Phi)
            {   
                ValuePtr incomingVal = nullptr;
                auto phi = dynamic_cast<PhiInstruction*>(bi->a->I);
                bool flag = false;
                for (auto iter : phi->from)
                {
                    auto val = iter.first;
                    auto pred = iter.second;
                    if (pred == latch)
                        flag = true;
                    
                    
                }
                if(!latch || !flag)
                    continue;
                bi->setOperands(0,CurrentIncomingValue[phi->reg]);
            }

            if(bi->b&&bi->b->I && bi->b->I->type ==Phi)
            {   
                ValuePtr incomingVal = nullptr;
                auto phi = dynamic_cast<PhiInstruction*>(bi->b->I);
                bool flag = false;
                for (auto iter : phi->from)
                {
                    auto val = iter.first;
                    auto pred = iter.second;
                    if (pred == latch)
                        flag = true;
                }
                if(!latch || !flag)
                    continue;
                bi->setOperands(1,CurrentIncomingValue[phi->reg]);
            }
        }
        else
            continue;
    }

    for(auto instr : headerPhi)
    {
        auto phi  = (PhiInstruction*)(instr.get());
        for(auto iter : phi->from)
        {
            auto val = iter.first;
            auto pred = iter.second;
            if(pred == loop->preHeader)
            {
                replaceVarByVar(phi->reg, val);
                
                unuseall(instr);
                phi->basicblock->removeInsturction(instr);
                break;
            }
        }
    }

    int iter_num = 0;

    auto block_iter = func->basicBlocks.begin();

    for(auto bb: func->basicBlocks)
    {
        block_iter++;
        if(bb == latch)
        {
            break;
        }
    }

    vector<BasicBlockPtr> ins_list;

    for(auto iter : latchHeaderEdge)
    {
        iter_num ++;

        auto latch_cp = iter;
        if(iter_num==latchHeaderEdge.size())
        {
            break;
        }
        auto nextLatch = latchHeaderEdge[iter_num];
        latch_cp->succBasicBlocks.insert(nextLatch);
        nextLatch->predBasicBlocks.clear();
        nextLatch->predBasicBlocks.insert(latch_cp);

        auto br = InstructionPtr( new BrInstruction(nextLatch->label, latch_cp));
        latch_cp->pushInstruction(br);

        if(!loop->contains(latch_cp)&&latch_cp) {
            loop->addBasicBlock(latch_cp);
            ins_list.push_back(latch_cp);
        }
    }

    func->basicBlocks.insert(block_iter,ins_list.begin(),ins_list.end());


    itr = --header->instructions.end();
    header->instructions.erase(itr);
    header->endInstruction = nullptr;
    auto br = InstructionPtr( new BrInstruction(latch->label, header));
    header->pushInstruction(br);

    for(auto iter: header->predBasicBlocks)
    {
        if(iter == latch)
        {
            header->predBasicBlocks.erase(iter);
            break;
        }
    }

    for(auto iter: exit->predBasicBlocks)
    {
        if(iter == header)
        {
            exit->predBasicBlocks.erase(iter);
            break;
        }
    }

    unordered_set<BasicBlockPtr> visited;

    for(auto bb: func->basicBlocks)
    {
        visited.insert(bb);
    }

    for(auto bb: func->basicBlocks)
    {
        for(auto iter: bb->getPredecessor())
        {
            if(!visited.count(iter))
                bb->predBasicBlocks.erase(iter);
        }
    }
}

// @FIXME: bug when compiling 09_BFS and almost useless
void unrollLoop(FunctionPtr func) {

    queue<LoopPtr> QueWorkList;
    for(auto loop : func->getAllLoops())
    {
        if(loop->getParentLoop()!=nullptr)
            continue;
        else
            QueWorkList.push(loop);
    }
    while(!QueWorkList.empty())
    {
        auto loop = QueWorkList.front();
        QueWorkList.pop();
        runOnLoop(loop, func);
        for(auto subLoop : loop->getSubLoops())
            QueWorkList.push(subLoop);
    }
}