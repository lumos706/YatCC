#include "constPro.h"
#include "LICM.h"


bool constPropogationImpl(FunctionPtr func){
    int regCount = 0;
    int removeCount = 0;
    bool Change = false;
    for(int bbIdx = 0; bbIdx < func->basicBlocks.size(); bbIdx ++) {
        auto bb = func->basicBlocks[bbIdx];
        unordered_map<InstructionPtr, BasicBlockPtr> del;
        unordered_map<BasicBlockPtr, bool> changedBB;

        for(int i = 0; i < bb->instructions.size(); i++){
            auto Ins = bb->instructions[i];

            if(Ins ->type == Binary)
            {
                auto Bi = (BinaryInstruction*)(Ins.get());
                if(Bi->a->isConst && Bi->b->isConst)
                {
                    auto op = Bi->op;
                    auto lhs = dynamic_cast<Const *>(Bi->a.get());
                    auto rhs = dynamic_cast<Const *>(Bi->b.get());
                    if(Bi->a->type->isFloat()||Bi->b->type->isFloat())
                    {
                        float myres = 0.0;
                        float lhs_value, rhs_value;
                        if (Bi->a->type->isFloat())
                            lhs_value = lhs->floatVal;
                        else
                            lhs_value = lhs->intVal * 1.0;
                        if (Bi->b->type->isFloat())
                            rhs_value = rhs->floatVal;
                        else
                            rhs_value = rhs->intVal * 1.0;
                        
                        if(op == '+')
                            myres = lhs_value + rhs_value;
                        else if(op == '-')
                            myres = lhs_value - rhs_value;
                        else if(op == '*')
                            myres = lhs_value * rhs_value;
                        else if(op == '/')
                            myres = lhs_value / rhs_value;
                        else 
                            continue;
                        
                        auto res = Const::getConst(Type::getFloat(),(float)myres, "constPropogation%" + to_string(regCount++));
                        replaceVarByVar(Bi->reg, res);
                        deleteUser(Bi->reg);
                    }
                    else if(Bi->a->type->isBool()||Bi->b->type->isBool())
                    {
                        bool myres = false;
                        bool lhs_value, rhs_value;

                        lhs_value = lhs->boolVal;
                        rhs_value = rhs->boolVal;
                        if(op == '+')
                            myres = lhs_value + rhs_value;
                        else if(op == '-')
                            myres = lhs_value - rhs_value;
                        else if(op == '*')
                            myres = lhs_value * rhs_value;
                        else if(op == '/')
                            myres = lhs_value / rhs_value;
                        else if(op == '%')
                            myres = lhs_value % rhs_value;
                        else if(op == '!')
                            myres = !lhs_value;
                        else if(op == ',')
                            myres = lhs_value << rhs_value;
                        else if(op == '.')
                            myres = lhs_value >> rhs_value;
                        else
                            continue;
                        auto res = Const::getConst(Type::getBool(),(bool)myres, "constPropogation%" + to_string(regCount++));

                        replaceVarByVar(Bi->reg, res);
                        deleteUser(Bi->reg);
                    }
                    else
                    {
                        int myres = 0.0;
                        int lhs_value, rhs_value;

                        // lhs_value = lhs->intVal;
                        // rhs_value = rhs->intVal;
                        lhs_value = lhs->type->isBool() ? lhs->boolVal : lhs->intVal;
                        rhs_value = rhs->type->isBool() ? rhs->boolVal : rhs->intVal;
                        if(op == '+')
                            myres = lhs_value + rhs_value;
                        else if(op == '-')
                            myres = lhs_value - rhs_value;
                        else if(op == '*')
                            myres = lhs_value * rhs_value;
                        else if(op == '/')
                            myres = lhs_value / rhs_value;
                        else if(op == '%')
                            myres = lhs_value % rhs_value;
                        else if(op == '!')
                            // myres = lhs_value ^ rhs_value;
                            myres = !lhs_value;
                        else if(op == ',')
                            myres = lhs_value << rhs_value;
                        else if(op == '.')
                            myres = lhs_value >> rhs_value;
                        else
                            continue;
                        auto res = Const::getConst(Type::getInt(),(int)myres, "constPropogation%" + to_string(regCount++));

                        replaceVarByVar(Bi->reg, res);
                        deleteUser(Bi->reg);
                    }

                    changedBB[bb]=true;
                    del[Ins] = bb;
                    
                }
            }
            else if(Ins->type == Icmp){
                auto Ii = dynamic_cast<IcmpInstruction*>(Ins.get());
                if(Ii->a->isConst&&Ii->b->isConst){
                    bool res = false;
                    //不可能是float，如果是，那就有bug
                    assert(!Ii->a->type->isFloat()&&!Ii->b->type->isFloat()&&"Icmp with float value");
                    int aVal = Ii->a->type->isInt()?dynamic_cast<Const*>(Ii->a.get())->intVal:dynamic_cast<Const*>(Ii->a.get())->boolVal;
                    int bVal = Ii->b->type->isInt()?dynamic_cast<Const*>(Ii->b.get())->intVal:dynamic_cast<Const*>(Ii->b.get())->boolVal;
                    string op = Ii->op;
                    if (op == "!=")
                    {
                        res = aVal!=bVal;
                    }
                    else if (op == ">")
                    {
                        res = aVal>bVal;
                    }
                    else if (op == ">=")
                    {
                        res = aVal>=bVal;
                    }
                    else if (op == "<")
                    {
                        res = aVal<bVal;
                    }
                    else if (op == "<=")
                    {
                        res = aVal<=bVal;
                    }
                    else if (op == "==")
                    {
                        res = aVal==bVal;
                    }
                    else{
                        assert(false&&"wrong op");
                    }
                    auto newConst = Const::getConst(Type::getBool(),res);

                    replaceVarByVar(Ii->reg, newConst);
                    deleteUser(Ii->reg);
                    changedBB[bb]=true;
                    del[Ins] = bb;
                }

            }
            else if(Ins->type == Fcmp){
                auto Fi = dynamic_cast<FcmpInstruction*>(Ins.get());
                if(Fi->a->isConst&&Fi->b->isConst){
                    bool res = false;
                    //不可能是int，如果是，那就有bug
                    assert(!Fi->a->type->isInt()&&!Fi->b->type->isInt()&&"Icmp with float value");
                    double aVal = Fi->a->type->isFloat()?dynamic_cast<Const*>(Fi->a.get())->floatVal:dynamic_cast<Const*>(Fi->a.get())->boolVal;
                    double bVal = Fi->b->type->isFloat()?dynamic_cast<Const*>(Fi->b.get())->floatVal:dynamic_cast<Const*>(Fi->b.get())->boolVal;
                    string op = Fi->op;
                    if (op == "!=")
                    {
                        res = aVal!=bVal;
                    }
                    else if (op == ">")
                    {
                        res = aVal>bVal;
                    }
                    else if (op == ">=")
                    {
                        res = aVal>=bVal;
                    }
                    else if (op == "<")
                    {
                        res = aVal<bVal;
                    }
                    else if (op == "<=")
                    {
                        res = aVal<=bVal;
                    }
                    else if (op == "==")
                    {
                        res = aVal==bVal;
                    }
                    else{
                        assert(false&&"wrong op");
                    }
                    auto newConst = Const::getConst(Type::getBool(),res);

                    replaceVarByVar(Fi->reg, newConst);
                    deleteUser(Fi->reg);
                    changedBB[bb]=true;
                    del[Ins] = bb;
                }
            }
            else if(Ins->type == Br){
                auto Bi = dynamic_cast<BrInstruction*>(Ins.get());
                if(Bi->exp&&Bi->exp->isConst){
                    Change = true;
                    auto C = dynamic_cast<Const*>(Bi->exp.get());
                    if(C->type->isBool()){
                        if( C->boolVal){
                            Bi->exp = nullptr;
                            Bi->label_false = nullptr;
                        }
                        else{
                            Bi->exp = nullptr;
                            Bi->label_true = Bi->label_false;
                            Bi->label_false = nullptr;
                        }
                    }
                    else if(C->type->isInt()){
                        if( C->intVal){
                            Bi->exp = nullptr;
                            Bi->label_false = nullptr;
                        }
                        else{
                            Bi->exp = nullptr;
                            Bi->label_true = Bi->label_false;
                            Bi->label_false = nullptr;
                        }
                    }
                    else if(C->type->isFloat()){
                        if( C->floatVal){
                            Bi->exp = nullptr;
                            Bi->label_false = nullptr;
                        }
                        else{
                            Bi->exp = nullptr;
                            Bi->label_true = Bi->label_false;
                            Bi->label_false = nullptr;
                        }
                    }
                }
            }
            else if(Ins->type == Phi){
                auto Pi = dynamic_cast<PhiInstruction*>(Ins.get());
                if(Pi->from.size()==1){
                    replaceVarByVar(Pi->reg,Pi->from[0].first);
                    changedBB[bb]=true;
                    del[Ins] = bb;
                }
            }
            else if(Ins->type == Ext) {
                auto ext = dynamic_cast<ExtInstruction *>(Ins.get());
                if(ext->from->isConst) {
                    ValuePtr newConst ;
                    if(ext->reg->type == Type::getInt()){
                        newConst = Const::getConst(Type::getInt(),dynamic_cast<Const*>(ext->from.get())->boolVal);
                    }
                    else if(ext->reg->type == Type::getInt64()){
                        newConst = Const::getConst(Type::getInt64(),dynamic_cast<Const*>(ext->from.get())->intVal);
                    }
                    replaceVarByVar(ext->reg, newConst);
                    deleteUser(ext->reg);
                    changedBB[bb]=true;
                    del[Ins] = bb;
                }
            }
            //这个先不做
            else if(Ins->type == Fneg){
                auto fneg = dynamic_cast<FnegInstruction *>(Ins.get());
                assert(fneg->a->type->isFloat());
                if(fneg->a->isConst) {
                    double floatVal = dynamic_cast<Const *>(fneg->a.get())->floatVal;
                    floatVal = -floatVal;
                    ValuePtr newConst = Const::getConst(Type::getFloat(), (float)floatVal, "constPropogation%" + to_string(regCount++));
                    replaceVarByVar(fneg->reg, newConst);
                    deleteUser(fneg->reg);
                    changedBB[bb]=true;
                    del[Ins] = bb;
                }
            }

        }
        if(del.size()>0){
            Change = true;
        }
        for(auto newBBPair: changedBB) {
            auto newBB = newBBPair.first;
            vector<InstructionPtr > newIns;
            for(int i = 0; i < newBB->instructions.size(); i++){
                if(del.find(newBB->instructions[i]) == del.end()){
                    newIns.push_back(newBB->instructions[i]);
                }
                else{
                    deleteUser(newBB->instructions[i].get());
                }
            }
            newBB->instructions = newIns;
        }
    }
    //因为修改了Br，所以导致CFG改变，有些块不会再跳转到另外的一些块中
    computeCFG(func);
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
    return Change;
}

// @FIXME: bug when compiling 09_BFS and almost useless
void constPropogation(FunctionPtr func) {
    while(constPropogationImpl(func)){}
}

