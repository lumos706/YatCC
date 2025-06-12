#include "SCEV.h"


void BIV2SCEV(LoopPtr loop, PhiInstruction* phi, std::set<ValuePtr>& adds, std::set<ValuePtr>& subs)
{
    // cerr << "BIV2SCEV" << endl;
    ValuePtr initial = nullptr;
    for(auto itr : phi->from)
    {   
        auto val = itr.first;
        if(loop->isSimpleLoopInvariant(val))
            initial = val;
    }

    // cerr << "BIV2SCEV passed step 1" << endl;
    
    if(!initial)
        return;
    if(!adds.empty())
    {
        // cerr << "BIV2SCEV passed step 2" << endl;
        auto step = *adds.begin();
        adds.erase(step);
        std::set<BinaryInstruction*> binaryStore;
        for(auto sub : subs)
        {
            // auto binary = new BinaryInst(step->getType(), Instruction::Sub, step, sub, nullptr, "scevsubtmp");
            auto binary = new BinaryInstruction(step,sub,'-',BasicBlockPtr(nullptr));
            binaryStore.insert(binary);
            step = binary->reg;
            // step = binary;
        }
        for(auto add : adds)
        {
            // auto binary = new BinaryInst(step->getType(), Instruction::Add, step, add, nullptr, "scevaddtmp");
            auto binary = new BinaryInstruction(step,add,'+',BasicBlockPtr(nullptr));
            binaryStore.insert(binary);
            step = binary->reg;
            // step = binary;
        }
        // cerr << "Init is "<< initial->getStr() << " Step is " << step->getStr() << endl;
        SCEV scev(initial, step, std::move(binaryStore));
        loop->registerSCEV((Instruction*)phi, scev);
        // cerr << "Finish ADD BIV2SCEV" << endl;
        return;
    }

    if(subs.empty())
    {
        return;
    }

    // cerr << "BIV2SCEV passed step 2" << endl;
    // cerr << "subset is " << subs.size() << endl;
    auto sub = *subs.begin();
    subs.erase(sub);
    // cerr << "BIV2SCEV passed step 3" << endl;
    std::set<BinaryInstruction*> binaryStore;
    int myval =0;
    auto step = new BinaryInstruction(ValuePtr(new Const(myval, "scevsubtmp")),sub,'-',BasicBlockPtr(nullptr));
    // cerr << "BIV2SCEV passed step 4" << endl;
    // auto step = new BinaryInst(Type::getInt32Ty(), Instruction::Sub, ConstantInt::getZero(Type::getInt32Ty()), sub, nullptr, "scevsubtmp");
    binaryStore.insert(step);
    for(auto sub : subs)
    {
        step = new BinaryInstruction(ValuePtr(new Const(myval, "scevsubtmp")),sub,'-',BasicBlockPtr(nullptr));
        binaryStore.insert(step);
    }
    // cerr << "BIV2SCEV passed step 5" << endl;
    SCEV scev(initial, step->reg, std::move(binaryStore));
    loop->registerSCEV(phi, scev);
    // cerr << "Finish BIV2SCEV" << endl;
}


void registerBIV(LoopPtr loop, PhiInstruction* phi)
{

    // cerr << "enter register BIV of phi" << phi->reg->getStr() <<endl;
    stack<BinaryInstruction *> workStk;
    for (auto use : phi->from)
    {
        if(use.first->isConst)
            continue;
        auto ptr = use.first->useHead;
        // assert(use.first->I !=nullptr);
        if(use.first->I == nullptr)
            continue;
        if(use.first->I ->type == Binary && use.first->I->basicblock)
        {
            auto instr = dynamic_cast<BinaryInstruction *>(use.first->I);
            // cerr << "name is " << instr->getName() << endl;
            workStk.push(instr);
        }
            
    }

    // cerr << "Finish register BIV step 1 of" << phi->reg->getStr() << endl;


    std::set<ValuePtr> addInSCEV;
    std::set<ValuePtr> subInSCEV;

    while (!workStk.empty())
    {
        auto binary = workStk.top();
        workStk.pop();

        if (binary->op == '+')
        {
            if (loop->isSimpleLoopInvariant(binary->a))
                addInSCEV.insert(binary->a);
            else if (loop->isSimpleLoopInvariant(binary->b))
                addInSCEV.insert(binary->b);
            else
                return;
        }
        else if (binary->op == '-')
        {
            if (loop->isSimpleLoopInvariant(binary->b))
                subInSCEV.insert(binary->b);
            else
                return;
        }
        auto reg = binary->reg;
        // cerr << "binary reg: " << reg->getStr() << " phi reg: " << phi->reg->getStr() << endl;
        auto ptr = reg->useHead;
        while (ptr != nullptr)
        {
            auto user = ptr->user;
            // user->print();
            // cerr << "instr type is " << user->type << endl;
            if (user->type == Phi)
            {
                
                // cerr << "has phi here" << endl;
                auto instr = dynamic_cast<PhiInstruction *>(user);
                // to be fixed
                if (user->reg == phi->reg)
                {
                    BIV2SCEV(loop, phi, addInSCEV, subInSCEV);
                    return;
                }
            }
            else if (user->type == Binary)
            {
                auto instr = dynamic_cast<BinaryInstruction *>(user);
                if (instr->op == '+' || instr->op == '-')
                {
                    workStk.push(instr);
                }
            }
            ptr = ptr ->next;
        }
    }
    // cerr << "Finish step 2 of findandReg" <<endl;
}

void findAndRegisterBIV(LoopPtr loop)
{

    // cerr << "enter find and register biv" << endl;
    auto header = loop->getHeader();
    std::vector<PhiInstruction*> phiList;
    for(auto instr : header->instructions)
    {
        
        if(instr->type == Phi)
        {
            auto phi = dynamic_cast<PhiInstruction*>(instr.get());
            phiList.push_back(phi);
        }   
        else
            break;
    }

    for(auto phi : phiList)
    {
        std::stack<Instruction*> workStk;
        // cerr << "find and reg handling " << phi->reg->getStr() << endl;
        for(auto use : phi->from)
        {
            auto ptr = use.first->useHead;
            while(ptr != nullptr)
            {
                auto user = ptr->user;
                assert(user != nullptr);
                workStk.push(user);
                ptr = ptr->next;
            }
        }
        // cerr << "find and reg handling " << phi->reg->getStr() << "step 2" << endl;

        while(!workStk.empty())
        {
            auto instr = workStk.top();
            workStk.pop();
            if(instr == phi)
            {
                // cerr << "Registering" << phi->reg->getStr() << endl;
                registerBIV(loop, phi);
                break;
            }

            if(auto phi = dynamic_cast<PhiInstruction*>(instr))
                break;
            

            // unfinished
            // if(loop->isSimpleLoopInvariant(instr))
            //     continue;
            

            auto val = instr->reg;
            if(val ==nullptr)
                continue;
            auto ptr = val->useHead;
            while(ptr != nullptr)
            {
                auto user = ptr->user;
                assert(user != nullptr);
                workStk.push(user);
                ptr = ptr->next;
            }
        }
    }
}

void linkInstructionToSCEV(LoopPtr loop, BinaryInstruction* binary, bool &fixed)
{

    // cerr << "Linking to SCEV" << endl;
    auto lhs = binary->a;
    auto rhs = binary->b;

    auto lhsInstr = lhs-> I;
    auto rhsInstr = rhs-> I;

    if(loop->hasSCEV(lhsInstr))
    {
        // cerr << "In SCEV "<<endl;
        if(loop->isSimpleLoopInvariant(rhs))
        {
            // cerr << "rhs is simple loop" << endl;
            if(binary->op == '+')
            {
                // cerr << "not pass single add" << endl;
                // cerr << "[passed] get scev " << endl;
                auto scev = rhs + loop->getSCEV(lhsInstr);
                // cerr << "[passed] simple add "<< endl;
                loop->registerSCEV(binary, scev);
                fixed = false;
                return;
            }
            else if(binary->op == '*')
            {
                auto scev = rhs * loop->getSCEV(lhsInstr);
                loop->registerSCEV(binary, scev);
                fixed = false;
                return;
            }
            else if(binary->op == '-')
            {
                auto scev = loop->getSCEV(lhsInstr) - rhs;
                loop->registerSCEV(binary, scev);
                fixed = false;
                return;
            }
        }
        else if(loop->hasSCEV(rhsInstr))
        {
            if(binary->op == '+')
            {
                auto scev = loop->getSCEV(lhsInstr) + loop->getSCEV(rhsInstr);
                loop->registerSCEV(binary, scev);
                fixed = false;
                return;
            } // TODO solve the Higher SCEV like {0, +, 1} * {0, +, 1} = {0, +, 1, +, 2}
            // else if(binary->getKind() == Kind::Mul)
            // {
            //     auto scev = loop->getSCEV(lhsInstr) * loop->getSCEV(rhsInstr);
            //     loop->registerSCEV(binary, scev);
            //     fixed = false;
            //     return;
            // }
            else if(binary->op == '-')
            {
                auto scev = loop->getSCEV(lhsInstr) - loop->getSCEV(rhsInstr);
                loop->registerSCEV(binary, scev);
                fixed = false;
                return;
            }
        }
    }
    else if(loop->hasSCEV(rhsInstr))
    {
        if(loop->isSimpleLoopInvariant(lhs))
        {
            if(binary->op == '+')
            {
                auto scev = lhs + loop->getSCEV(rhsInstr);
                loop->registerSCEV(binary, scev);
                fixed = false;
                return;
            }
            else if(binary->op == '*')
            {
                auto scev = lhs * loop->getSCEV(rhsInstr);
                loop->registerSCEV(binary, scev);
                fixed = false;
                return;
            }
        }
    }

}

void ScalarEvolution(LoopPtr loop)
{

    // cerr << "SCEV for loop" << endl;
    if(loop == nullptr)
        return;
    
    for(auto subLoop : loop->getSubLoops())
        ScalarEvolution(subLoop);

    loop->cleanSCEV();
    // Step1. Find the BIV.
    findAndRegisterBIV(loop);
    bool fixed = false;

    // cerr << "finish find and register" <<endl;

    // Step2. For every instrution calculate the SCEV
    do
    {
        fixed = true;

        for(auto bb : loop->getBasicBlocks())
        {
            for(auto instr : bb->instructions)
            {
                if(loop->hasSCEV(instr.get()))
                    continue;
                if(instr->type == Binary)
                {
                    // cerr << instr->getName()<< endl;
                    auto binary = dynamic_cast<BinaryInstruction *>(instr.get());
                    if(binary->op=='+'|| binary->op =='-'|| binary->op=='*')
                    {
                        linkInstructionToSCEV(loop, binary, fixed);
                    }
                }
            }
        }

    } while(!fixed);

    if(loop->indPhi && loop->hasSCEV(loop->indPhi))
    {

        // cerr << "indPhi" << endl;
        auto phi = loop->indPhi;
        auto& scev = loop->getSCEV(phi);
        auto endval = loop->indEnd;
        auto initval = scev.at(0);
        auto stepval = scev.at(1);
        if(endval && endval->isConst && initval && initval ->isConst && stepval && stepval->isConst)
        {
            // cerr << "May have trip count" << endl;
            auto cEnd = dynamic_cast<Const *>(endval.get());
            auto cInitial = dynamic_cast<Const *>(initval.get());
            auto cStep = dynamic_cast<Const *>(stepval.get());
            int c;
            if (loop->icmpKind == ICmpEQ || loop->icmpKind == ICmpSGE || loop->icmpKind == ICmpSLE)
                c = (cEnd->intVal + 1 - cInitial->intVal) / cStep->intVal;
            else
                c = (cEnd->intVal - cInitial->intVal) / cStep->intVal;
            // cerr << "loop trip count is" << c << endl;
            loop->tripCount = c;
        }
    }
}

