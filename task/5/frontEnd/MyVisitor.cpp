#include "MyVisitor.h"
#include <string>
#include <any>
using std::cout;
using std::endl;
using std::stoi;

void MyVisitor::print()
{
    irModule.print();
}

void printUse(ValuePtr val)
{
    Use *t = val->useHead;
    cout << "%" << val->name << " : User:\n";
    while (t)
    {
        t->user->print();
        t = t->next;
    }
}

void MyVisitor::opt()
{
    for (auto &func : (irModule.globalFunctions))
    {
        if (!func->isLib)
        {
            computeCFG(func);
            domTree(func);
            mem2reg(func);
        }
    }
    for (auto &func : (irModule.globalFunctions))
    {
        if (!func->isLib)
        {
            dce(func);
            reassociate(func);
        }
    }
    globalConstReplace(irModule);
    inliner(irModule);
    for (auto &func : (irModule.globalFunctions))
    {
        if (!func->isLib)
        {
            computeCFG(func);
            domTree(func);
            simplifyCFG(func);
            computeCFG(func);
            domTree(func);
            computeUse(func);
        }
    }

    for (auto &func : (irModule.globalFunctions))
    {
        if (!func->isLib)
        {
            loopAnalysis(func);
            dce(func);
            cse(func);
            strengthReduction(func);
            constPropogation(func);
            LICM(func);
            LCSSA(func);
            // unrollLoop(func);
            computeUse(func);
            reassociate(func);
            computeUse(func);
            dce(func);
            computeUse(func);
            // constPropogation(func);
        }
    }

    // for (auto &func : (irModule.globalFunctions))
    // {
    //     if (!func->isLib)
    //     {
    //         computeCFG(func);
    //         domTree(func);
    //         simplifyCFG(func);
    //         computeCFG(func);
    //         domTree(func);
    //         cse(func);
    //         computeUse(func);
    //         constPropogation(func);
    //         computeCFG(func);
    //         domTree(func);
    //     }
    // }
}

//访问CompUnit，即整个代码
antlrcpp::Any MyVisitor::visitCompUnit(SysY2022Parser::CompUnitContext *ctx)
{
    return visitChildren(ctx);
}


bool MyVisitor::dfsInitVal(SysY2022Parser::InitValContext *ctx)
{
    int sz = ctx->children.size();
    if (sz > 1)
    {
        for (int ind = 1; ind < sz - 1; ind += 2)
        {
            if (!dfsInitVal(dynamic_cast<SysY2022Parser::InitValContext *>(ctx->children[ind])))
                return false;
        }
        return true;
    }
    else
    {
        try
        {
            stoi(dynamic_cast<SysY2022Parser::ItemInitValContext *>(ctx)->getText());
            return true;
        }
        catch (const std::exception &e)
        {
            return false;
        }
    }
}

antlrcpp::Any MyVisitor::visitItemConstInitVal(SysY2022Parser::ItemConstInitValContext *ctx)
{
    return ctx->constExp()->accept(this);
}

antlrcpp::Any MyVisitor::visitItemInitVal(SysY2022Parser::ItemInitValContext *ctx)
{
    return ctx->exp()->accept(this);
}

antlrcpp::Any MyVisitor::visitListInitVal(SysY2022Parser::ListInitValContext *ctx)
{
    return visitChildren(ctx);
}

antlrcpp::Any MyVisitor::visitFuncFParam(SysY2022Parser::FuncFParamContext *ctx)
{
    auto name = ctx->Identifier()->getText();
    auto type = TypePtr(new Type(ctx->bType()->getText()));
    if (ctx->children.size() == 2)
    {
        assert(type->isInt() || type->isFloat());
        if (type->isInt())
            return VariablePtr(new Int(name, false, false));
        else
            return VariablePtr(new Float(name, false, false));
    }
    else
    {
        auto curr = type;
        for (int childInd = ctx->children.size() - 2; childInd >= 5; childInd -= 3)
        {
            curr = TypePtr(new ArrType(curr, stoi(std::any_cast<ValuePtr>(ctx->children[childInd]->accept(this))->getStr())));
            // curr = TypePtr(new ArrType(curr, stoi(ctx->children[childInd]->accept(this).as<ValuePtr>()->getStr())));
        }  
        curr = TypePtr(new PtrType(curr));
        return VariablePtr(new Ptr(name, false, false, curr));
    }
}

antlrcpp::Any MyVisitor::visitFuncFParams(SysY2022Parser::FuncFParamsContext *ctx)
{
    vector<VariablePtr> rst;
    for (int i = 0; i < ctx->children.size(); i += 2)
    {
        rst.emplace_back(std::any_cast<VariablePtr>(ctx->children[i]->accept(this)));
        // rst.emplace_back(ctx->children[i]->accept(this).as<VariablePtr>());
    }
    return rst;
}

antlrcpp::Any MyVisitor::visitFuncDef(SysY2022Parser::FuncDefContext *ctx)
{
    auto type = TypePtr(new Type(ctx->funcType()->getText()));
    auto name = ctx->Identifier()->getText();

    irModule.pushFunc(FunctionPtr(new Function()));
    auto params = vector<ValuePtr>();
    if (ctx->children.size() == 6)
    {
        auto rst = std::any_cast<vector<VariablePtr>>(ctx->funcFParams()->accept(this));
        irModule.paramStringTable = StringTableNodePtr(new StringTableNode());
        for (auto var : rst)
        {
            // std::cerr<<var->getTypeStr()<<endl;
            params.emplace_back(var);
            auto addr = Variable::copy(var);
            addr->name += ".addr";
            irModule.paramStringTable->variableTable[var->name] = addr;
            irModule.getFunc()->pushVariable(addr);
            irModule.getBasicBlock()->pushInstruction(InstructionPtr(new StoreInstruction(addr, var, irModule.getBasicBlock())));
        }
    }
    auto func = FunctionPtr(new Function(type, name, params));
    func->setReturnBasicBlock();
    irModule.pushGlobalFunction(func);
    if (type->isInt())
    {
        VariablePtr retVal = VariablePtr(new Int("retval", false, false));
        irModule.getFunc()->pushVariable(retVal);
        func->retVal = retVal;
    }
    else if (type->isFloat())
    {
        VariablePtr retVal = VariablePtr(new Float("retval", false, false));
        irModule.getFunc()->pushVariable(retVal);
        func->retVal = retVal;
    }
    else
        func->retVal = Void::get();
    ctx->block()->accept(this);
    if (type->isVoid() || !irModule.getBasicBlock()->endInstruction)
        irModule.getBasicBlock()->setEndInstruction(InstructionPtr(new BrInstruction(irModule.globalFunctions.back()->returnBasicBlock->label, irModule.getBasicBlock())));
    
    irModule.getFunc()->allocLocalVariable();
    irModule.getFunc()->setBBbelongFunc(irModule.globalFunctions.back());

    auto temp_func = irModule.popFunc();
    auto& globalFunc = irModule.globalFunctions.back();

    globalFunc->regNum = temp_func->regNum;
    globalFunc->basicBlocks = temp_func->basicBlocks;
    globalFunc->variables = temp_func->variables;
    globalFunc->LabelBBMap = temp_func->LabelBBMap;
    globalFunc->topLoops = temp_func->topLoops;
    globalFunc->loops = temp_func->loops;
    globalFunc->bbToLoop = temp_func->bbToLoop;

    irModule.globalFunctions.back()->solveReturnBasicBlock();
    irModule.globalFunctions.back()->solveEndInstruction();
    return nullptr;
}

antlrcpp::Any MyVisitor::visitBlock(SysY2022Parser::BlockContext *ctx)
{
    irModule.pushBackStringTable();
    if (irModule.paramStringTable)
    {
        auto table = irModule.paramStringTable->variableTable;
        for (auto it = table.begin(); it != table.end(); it++)
        {
            irModule.currStringTable->variableTable[it->first] = it->second;
        }
        irModule.paramStringTable = nullptr;
    }
    visitChildren(ctx);
    irModule.popStringTable();
    return nullptr;
}

antlrcpp::Any MyVisitor::visitReturnStmt(SysY2022Parser::ReturnStmtContext *ctx)
{
    ValuePtr value;
    if (ctx->exp())
    {
        value = std::any_cast<ValuePtr>(ctx->exp()->accept(this));
        // value = ctx->exp()->accept(this).as<ValuePtr>();
        if (!value->type->operator==(irModule.globalFunctions.back()->retVal->type))
        {
            if (value->type->isFloat() && irModule.globalFunctions.back()->retVal->type->isInt())
            {
                if (value->isConst)
                {
                    // value->type = Type::getInt();
                    value = Const::getConst(Type::getInt(),int(dynamic_cast<Const *>(value.get())->floatVal), value->name);
                    // dynamic_cast<Const *>(value.get())->intVal = dynamic_cast<Const *>(value.get())->floatVal;
                }
                else
                {
                    auto instruction = shared_ptr<FptosiInstruction>(new FptosiInstruction(value, irModule.getBasicBlock()));
                    irModule.getBasicBlock()->pushInstruction(instruction);
                    value = instruction->reg;
                }
            }
            else if(value->type->isInt() && irModule.globalFunctions.back()->retVal->type->isFloat()){
                if(value->isConst){
                    // value->type = Type::getFloat();
                    // dynamic_cast<Const *>(value.get())->floatVal = dynamic_cast<Const *>(value.get())->intVal;
                    value = Const::getConst(Type::getFloat(),float(dynamic_cast<Const *>(value.get())->intVal), value->name);
                }
                else{
                    auto instruction = shared_ptr<SitofpInstruction>(new SitofpInstruction(value, irModule.getBasicBlock()));
                    irModule.getBasicBlock()->pushInstruction(instruction);
                    value = instruction->reg;
                }
            }
            // to-do
        }
        irModule.getBasicBlock()->pushInstruction(InstructionPtr(new StoreInstruction(irModule.globalFunctions.back()->retVal, value, irModule.getBasicBlock())));
    }
    else
    {
        value = Void::get();
    }
    irModule.getBasicBlock()->setEndInstruction(InstructionPtr(new BrInstruction(irModule.globalFunctions.back()->returnBasicBlock->label, irModule.getBasicBlock())));

    return nullptr;
}

/*
    变量定义时需要同时到 Block 和 符号表 中注册
    block是帮助后续内存分配使用的，注册使用假名对应的指针
    符号表是帮助变量访问使用的， key 为真名， value 为假名对应的指针
    假名在不重复时同真名，否则为真名加上一个 hash 值
*/

antlrcpp::Any MyVisitor::visitVarDefWithOutInitVal(SysY2022Parser::VarDefWithOutInitValContext *ctx)
{
    auto name = ctx->Identifier()->getText();
    int childSize = ctx->children.size();
    // to-do: type
    if (irModule.blockStackEmpty()) // 全局变量
    {
        if (childSize == 1)
        {
            if (irModule.declType->isInt())
                irModule.pushVariable(VariablePtr(new Int(name, true, false)));
            else if (irModule.declType->isFloat())
                irModule.pushVariable(VariablePtr(new Float(name, true, false)));
        }
        else
        {
            auto curr = shared_ptr<ArrType>(new ArrType(irModule.declType, stoi(std::any_cast<ValuePtr>(ctx->children[childSize - 2]->accept(this))->getStr())));
            // auto curr = shared_ptr<ArrType>(new ArrType(irModule.declType, stoi(ctx->children[childSize - 2]->accept(this).as<ValuePtr>()->getStr())));
            for (int childInd = childSize - 5; childInd > 1; childInd -= 3)
            {
                curr = shared_ptr<ArrType>(new ArrType(curr, stoi(std::any_cast<ValuePtr>(ctx->children[childInd]->accept(this))->getStr())));
                // curr = shared_ptr<ArrType>(new ArrType(curr, stoi(ctx->children[childInd]->accept(this).as<ValuePtr>()->getStr())));
            }
            VariablePtr variable = shared_ptr<Arr>(new Arr(name, true, false, curr));
            irModule.pushVariable(variable);
        }
    }
    else // 局部变量
    {
        if (childSize == 1)
        {
            VariablePtr variable;
            if (irModule.declType->isInt())
                variable = VariablePtr(new Int(name, false, false));
            else if (irModule.declType->isFloat())
                variable = VariablePtr(new Float(name, false, false));
            irModule.registerVariable(variable);
        }
        else
        {
            auto curr = shared_ptr<ArrType>(new ArrType(irModule.declType, stoi(std::any_cast<ValuePtr>(ctx->children[childSize - 2]->accept(this))->getStr())));
            // auto curr = shared_ptr<ArrType>(new ArrType(irModule.declType, stoi(ctx->children[childSize - 2]->accept(this).as<ValuePtr>()->getStr())));
            for (int childInd = childSize - 5; childInd > 1; childInd -= 3)
            {
                curr = shared_ptr<ArrType>(new ArrType(curr, stoi(std::any_cast<ValuePtr>(ctx->children[childInd]->accept(this))->getStr())));
                // curr = shared_ptr<ArrType>(new ArrType(curr, stoi(ctx->children[childInd]->accept(this).as<ValuePtr>()->getStr())));
            }
            VariablePtr variable = shared_ptr<Arr>(new Arr(name, false, false, curr));
            irModule.registerVariable(variable);
        }
    }

    return visitChildren(ctx);
}

VariablePtr MyVisitor::arrInitItem(SysY2022Parser::ItemInitValContext *ctx, TypePtr type)
{
    auto number = std::any_cast<ValuePtr>(dynamic_cast<SysY2022Parser::ItemInitValContext *>(ctx)->accept(this));
    // auto number = dynamic_cast<SysY2022Parser::ItemInitValContext *>(ctx)->accept(this).as<ValuePtr>();
    if (irModule.declType->isInt())
        return VariablePtr(new Int("", false, false, number));
    else
        return VariablePtr(new Float("", false, false, number));
}

shared_ptr<Arr> MyVisitor::arrInitList(SysY2022Parser::ListInitValContext *ctx, TypePtr type)
{
    int sz = ctx->children.size();
    shared_ptr<Arr> rst = shared_ptr<Arr>(new Arr(string("__const.") + to_string(constArr++), true, true, type));
    for (int ind = 1; ind < sz - 1; ind += 2)
    {
        VariablePtr curr;
        if (ctx->children[ind]->children.size() > 1)
        {
            auto tmp = arrInitList(dynamic_cast<SysY2022Parser::ListInitValContext *>(ctx->children[ind]), rst->getElementType());
            tmp->fill();
            curr = tmp;
        }
        else
            curr = arrInitItem(dynamic_cast<SysY2022Parser::ItemInitValContext *>(ctx->children[ind]), rst->getElementType());
        rst->push(curr);
    }
    return rst;
}

VariablePtr MyVisitor::arrInitItem(SysY2022Parser::ItemConstInitValContext *ctx, TypePtr type)
{
    string number = dynamic_cast<SysY2022Parser::ItemConstInitValContext *>(ctx)->getText();
    assert(!number.empty());
    return VariablePtr(new Int("", false, false, stoi(number)));
}

shared_ptr<Arr> MyVisitor::arrInitList(SysY2022Parser::ListConstInitValContext *ctx, TypePtr type)
{
    int sz = ctx->children.size();
    shared_ptr<Arr> rst = shared_ptr<Arr>(new Arr("", true, true, type));
    for (int ind = 1; ind < sz - 1; ind += 2)
    {
        VariablePtr curr;
        if (ctx->children[ind]->children.size() > 1)
        {
            auto tmp = arrInitList(dynamic_cast<SysY2022Parser::ListConstInitValContext *>(ctx->children[ind]), rst->getElementType());
            tmp->fill();
            curr = tmp;
        }
        else
            curr = arrInitItem(dynamic_cast<SysY2022Parser::ItemConstInitValContext *>(ctx->children[ind]), rst->getElementType());
        rst->push(curr);
    }
    return rst;
}

bool indexAdd(vector<ValuePtr> &indexs, int depth, TypePtr type)
{
    if (depth == indexs.size() - 1)
    {
        int oldIndex = dynamic_cast<Const *>(indexs[depth].get())->intVal;
        assert(type->isArr());
        if (dynamic_cast<ArrType *>(type.get())->length - 1 == oldIndex)
        {
            return false;
        }
        else
        {
            // indexs[depth] = ValuePtr(new Const(Type::getInt64(), oldIndex + 1));
            indexs[depth] = Const::getConst(Type::getInt64(), (long long)(oldIndex + 1));
            return true;
        }
    }
    else
    {
        if (!indexAdd(indexs, depth + 1, dynamic_cast<ArrType *>(type.get())->inner))
        {
            int oldIndex = dynamic_cast<Const *>(indexs[depth].get())->intVal;
            if (dynamic_cast<ArrType *>(type.get())->length - 1 == oldIndex)
                return false;
            else
            {
                // indexs[depth] = ValuePtr(new Const(Type::getInt64(), oldIndex + 1));
                // indexs[depth + 1] = ValuePtr(new Const(Type::getInt64(), 0)); // 累进可能有bug
                indexs[depth] = Const::getConst(Type::getInt64(), (long long)(oldIndex + 1));
                indexs[depth + 1] = Const::getConst(Type::getInt64(), (long long)(0)); // 累进可能有bug
                return true;
            }
        }
        return true;
    }
}

void MyVisitor::arrSetList(SysY2022Parser::ListInitValContext *ctx, ValuePtr value)
{
    // int length = dynamic_cast<ArrType *>(value->type.get())->length;
    // if ((2 * length + 1) == ctx->children.size()) // 满的
    // { // llvm 形式的 set
    //     ValuePtr curr = value;
    //     for (int i = 1, ind = 0; i < ctx->children.size(); i += 2, ind++)
    //     {
    //         vector<ValuePtr> indexs;
    //         if (ind)
    //             indexs.emplace_back(ValuePtr(new Const(Type::getInt64(), 1)));
    //         else
    //         {
    //             indexs.emplace_back(ValuePtr(new Const(Type::getInt64(), 0)));
    //             indexs.emplace_back(ValuePtr(new Const(Type::getInt64(), 0)));
    //         }
    //         auto ins = shared_ptr<GetElementPtrInstruction>(new GetElementPtrInstruction(curr, indexs));
    //         irModule.getBasicBlock()->pushInstruction(ins);
    //         curr = ins->reg;
    //         if (ctx->children[i]->children.size() > 1)
    //             arrSetList(dynamic_cast<SysY2022Parser::ListInitValContext *>(ctx->children[i]), curr);
    //         else
    //         {
    //             auto reg = ctx->children[i]->accept(this).as<ValuePtr>();
    //             irModule.getBasicBlock()->pushInstruction(InstructionPtr(new StoreInstruction(curr, reg)));
    //         }
    //     }
    // }
    // else
    { // 暴力 set
        vector<ValuePtr> indexs;
        int depth = dynamic_cast<ArrType *>(value->type.get())->getDepth();
        for (int i = 0; i <= depth; i++)
                indexs.emplace_back(Const::getConst(Type::getInt64(), (long long)(0)));
            // indexs.emplace_back(ValuePtr(new Const(Type::getInt64(), 0)));
        if (ctx->children.size() == 2)
            return;
        for (int i = 1, ind = 0; i < ctx->children.size(); i += 2, ind++)
        {
            if (ctx->children[i]->children.size() == 1)
            { // 值
                auto reg = std::any_cast<ValuePtr>(ctx->children[i]->accept(this));
                auto ins = shared_ptr<GetElementPtrInstruction>(new GetElementPtrInstruction(value, indexs, irModule.getBasicBlock()));
                if (!value->isReg && dynamic_cast<Variable *>(value.get())->isGlobal)
                {
                    irModule.getBasicBlock()->pushInstruction(InstructionPtr(new StoreInstruction(ins, reg, irModule.getBasicBlock())));
                }
                else
                {
                    irModule.getBasicBlock()->pushInstruction(ins);
                    auto val = ins->reg;
                    if (!val->type->operator==(reg->type))
                    {
                        if (val->type->isInt() && reg->type->isFloat())
                        {
                            // to-do: isconst
                            auto ins = shared_ptr<FptosiInstruction>(new FptosiInstruction(reg, irModule.getBasicBlock()));
                            irModule.getBasicBlock()->pushInstruction(ins);
                            reg = ins->reg;
                        }
                        else if (val->type->isFloat() && reg->type->isInt())
                        {
                            // to-do: isconst
                            auto ins = shared_ptr<SitofpInstruction>(new SitofpInstruction(reg, irModule.getBasicBlock()));
                            irModule.getBasicBlock()->pushInstruction(ins);
                            reg = ins->reg;
                        }
                    }
                    irModule.getBasicBlock()->pushInstruction(InstructionPtr(new StoreInstruction(val, reg, irModule.getBasicBlock())));
                    // irModule.getBasicBlock()->instructions.back()->print();
                }

                indexAdd(indexs, 1, value->type);
            }
            else
            { // 列表
                vector<ValuePtr> inner = {indexs[0], indexs[1]};
                auto ins = shared_ptr<GetElementPtrInstruction>(new GetElementPtrInstruction(value, inner, irModule.getBasicBlock()));
                irModule.getBasicBlock()->pushInstruction(ins);
                arrSetList(dynamic_cast<SysY2022Parser::ListInitValContext *>(ctx->children[i]), ins->reg);
                // indexs[1] = ValuePtr(new Const(Type::getInt64(), dynamic_cast<Const *>(indexs[1].get())->intVal + 1));
                indexs[1] = Const::getConst(Type::getInt64(),(long long)( dynamic_cast<Const *>(indexs[1].get())->intVal + 1));
                for (int i = 2; i <= depth; i++)
                    indexs[i] = Const::getConst(Type::getInt64(), (long long)(0));
                    // indexs[i] = ValuePtr(new Const(Type::getInt64(), 0));
            }
        }
    }
}

antlrcpp::Any MyVisitor::visitVarDefWithInitVal(SysY2022Parser::VarDefWithInitValContext *ctx)
{
    auto name = ctx->Identifier()->getText();
    int childSize = ctx->children.size();

    if (irModule.blockStackEmpty()) // 全局变量
    {
        if (childSize == 3)
        {
            if (irModule.declType->isInt())
                irModule.pushVariable(VariablePtr(new Int(name, true, false, std::any_cast<ValuePtr>(ctx->initVal()->accept(this)))));
                // irModule.pushVariable(VariablePtr(new Int(name, true, false, ctx->initVal()->accept(this).as<ValuePtr>())));
            else if (irModule.declType->isFloat())
                // irModule.pushVariable(VariablePtr(new Float(name, true, false, ctx->initVal()->accept(this).as<ValuePtr>())));
                irModule.pushVariable(VariablePtr(new Float(name, true, false, std::any_cast<ValuePtr>(ctx->initVal()->accept(this)))));
        }
        else
        {
            auto curr = shared_ptr<ArrType>(new ArrType(irModule.declType, stoi(std::any_cast<ValuePtr>(ctx->children[childSize - 4]->accept(this))->getStr())));
            // auto curr = shared_ptr<ArrType>(new ArrType(irModule.declType, stoi(ctx->children[childSize - 4]->accept(this).as<ValuePtr>()->getStr())));
            for (int childInd = childSize - 7; childInd > 1; childInd -= 3)
            {
                curr = shared_ptr<ArrType>(new ArrType(curr, stoi(std::any_cast<ValuePtr>(ctx->children[childInd]->accept(this))->getStr())));
                // curr = shared_ptr<ArrType>(new ArrType(curr, stoi(ctx->children[childInd]->accept(this).as<ValuePtr>()->getStr())));
            }
            auto initVal = arrInitList(dynamic_cast<SysY2022Parser::ListInitValContext *>(ctx->initVal()), curr);
            auto variable = shared_ptr<Arr>(new Arr(name, true, false, curr));
            variable->inner = initVal->inner;
            irModule.pushVariable(variable);
        }
    }
    else // 局部变量
    {
        if (childSize == 3)
        {
            VariablePtr variable;
            if (irModule.declType->isInt())
                variable = VariablePtr(new Int(name, false, false));
            else if (irModule.declType->isFloat())
                variable = VariablePtr(new Float(name, false, false));
            auto exp = std::any_cast<ValuePtr>(ctx->initVal()->accept(this));
            // auto exp = ctx->initVal()->accept(this).as<ValuePtr>();
            if(exp->type->ID!=variable->type->ID){
                //int->float
                if(exp->type->isInt()){
                    auto instruction = shared_ptr<SitofpInstruction>(new SitofpInstruction(exp, irModule.getBasicBlock()));
                    irModule.getBasicBlock()->pushInstruction(instruction);
                    exp = instruction->reg;
                }
                //float->int
                else{
                    auto instruction = shared_ptr<FptosiInstruction>(new FptosiInstruction(exp, irModule.getBasicBlock()));
                    irModule.getBasicBlock()->pushInstruction(instruction);
                    exp = instruction->reg;
                }
            }
            
            irModule.getBasicBlock()->pushInstruction(InstructionPtr(new StoreInstruction(variable, exp, irModule.getBasicBlock())));
            irModule.registerVariable(variable);
        }
        else // 数组
        {
            auto curr = shared_ptr<ArrType>(new ArrType(irModule.declType, stoi(std::any_cast<ValuePtr>(ctx->children[childSize - 4]->accept(this))->getStr())));
            
            // auto curr = shared_ptr<ArrType>(new ArrType(irModule.declType, stoi(ctx->children[childSize - 4]->accept(this).as<ValuePtr>()->getStr())));
            for (int childInd = childSize - 7; childInd > 1; childInd -= 3)
            {
                curr = shared_ptr<ArrType>(new ArrType(curr, stoi(std::any_cast<ValuePtr>(ctx->children[childInd]->accept(this))->getStr())));
                // curr = shared_ptr<ArrType>(new ArrType(curr, stoi(ctx->children[childInd]->accept(this).as<ValuePtr>()->getStr())));
            }

            VariablePtr variable = shared_ptr<Arr>(new Arr(name, false, false, curr));
            irModule.registerVariable(variable);

            bool isConstInitVal = dfsInitVal(ctx->initVal());

            if (isConstInitVal)
            {
                // 初始值作为全局数组存储
                // auto arrBitCast = shared_ptr<BitCastInstruction>(new BitCastInstruction(variable, irModule.getFunc()->getReg(TypePtr(new PtrType(Type::getInt8()))), irModule.getBasicBlock()));
                // irModule.getBasicBlock()->pushInstruction(arrBitCast);
                // auto initVal = arrInitList(dynamic_cast<SysY2022Parser::ListInitValContext *>(ctx->initVal()), curr);
                // if (!initVal->zero())
                // {
                //     irModule.globalVariables.emplace_back(initVal);
                //     auto initValBitCast = shared_ptr<BitCastInstruction>(new BitCastInstruction(initVal, irModule.getFunc()->getReg(TypePtr(new PtrType(Type::getInt8()))), irModule.getBasicBlock()));
                //     irModule.getBasicBlock()->pushInstruction(initValBitCast);
                //     vector<ValuePtr> argv = {arrBitCast->reg,
                //                              initValBitCast->reg,
                //                              ValuePtr(new Const(Type::getInt64(), dynamic_cast<ArrType *>(variable->type.get())->getSize() * 4)),
                //                              ValuePtr(new Const(false))};
                //     irModule.getBasicBlock()->pushInstruction(InstructionPtr(new CallInstruction(irModule.globalFunctions[1], argv, irModule.getBasicBlock())));
                // }
                // else
                // {
                //     vector<ValuePtr> argv = {arrBitCast->reg,
                //                              ValuePtr(new Const(Type::getInt8(), 0)),
                //                              ValuePtr(new Const(Type::getInt64(), dynamic_cast<ArrType *>(variable->type.get())->getSize() * 4)),
                //                              ValuePtr(new Const(false))};
                //     irModule.getBasicBlock()->pushInstruction(InstructionPtr(new CallInstruction(irModule.globalFunctions[0], argv, irModule.getBasicBlock())));
                // }

                // 暴力 set
                auto arrBitCast = shared_ptr<BitCastInstruction>(new BitCastInstruction(variable, irModule.getFunc()->getReg(TypePtr(new PtrType(Type::getInt8()))), irModule.getBasicBlock()));
                irModule.getBasicBlock()->pushInstruction(arrBitCast);
                // vector<ValuePtr> argv = {arrBitCast->reg,
                //                          ValuePtr(new Const(Type::getInt8(), 0)),
                //                          ValuePtr(new Const(Type::getInt64(), dynamic_cast<ArrType *>(variable->type.get())->getSize() * 4)),
                //                          ValuePtr(new Const(false))};
                vector<ValuePtr> argv = {arrBitCast->reg,
                                         Const::getConst(Type::getInt8(), int8_t(0)),
                                         Const::getConst(Type::getInt64(),(long long)( dynamic_cast<ArrType *>(variable->type.get())->getSize() * 4)),
                                         Const::getConst(Type::getBool(),false)};
                irModule.getBasicBlock()->pushInstruction(InstructionPtr(new CallInstruction(irModule.globalFunctions[0], argv, irModule.getBasicBlock())));
                arrSetList(dynamic_cast<SysY2022Parser::ListInitValContext *>(ctx->initVal()), variable);
            }
            else
            {
                // 无脑 memset 0
                auto arrBitCast = shared_ptr<BitCastInstruction>(new BitCastInstruction(variable, irModule.getFunc()->getReg(TypePtr(new PtrType(Type::getInt8()))), irModule.getBasicBlock()));
                irModule.getBasicBlock()->pushInstruction(arrBitCast);
                // vector<ValuePtr> argv = {arrBitCast->reg,
                //                          ValuePtr(new Const(Type::getInt8(), 0)),
                //                          ValuePtr(new Const(Type::getInt64(), dynamic_cast<ArrType *>(variable->type.get())->getSize() * 4)),
                //                          ValuePtr(new Const(false))};
                vector<ValuePtr> argv = {arrBitCast->reg,
                                         Const::getConst(Type::getInt8(), 0),
                                         Const::getConst(Type::getInt64(), dynamic_cast<ArrType *>(variable->type.get())->getSize() * 4),
                                         Const::getConst(Type::getBool(),false)};
                irModule.getBasicBlock()->pushInstruction(InstructionPtr(new CallInstruction(irModule.globalFunctions[0], argv, irModule.getBasicBlock())));

                // 初始值一位一位 set
                arrSetList(dynamic_cast<SysY2022Parser::ListInitValContext *>(ctx->initVal()), variable);
            }
        }
    }

    return nullptr;
}

antlrcpp::Any MyVisitor::visitConstDecl(SysY2022Parser::ConstDeclContext *ctx)
{
    return visitChildren(ctx);
}

antlrcpp::Any MyVisitor::visitBType(SysY2022Parser::BTypeContext *ctx)
{
    irModule.declType = TypePtr(new Type(ctx->getText()));
    return nullptr;
}

antlrcpp::Any MyVisitor::visitConstDef(SysY2022Parser::ConstDefContext *ctx)
{
    auto name = ctx->Identifier()->getText();
    int childSize = ctx->children.size();

    // to-do: type
    if (irModule.blockStackEmpty()) // 全局变量
    {
        if (childSize == 3)
        {
            if (irModule.declType->isInt())
                irModule.pushVariable(VariablePtr(new Int(name, true, true, std::any_cast<ValuePtr>(ctx->constInitVal()->accept(this)))));
                // irModule.pushVariable(VariablePtr(new Int(name, true, true, ctx->constInitVal()->accept(this).as<ValuePtr>())));
            else if (irModule.declType->isFloat())
                irModule.pushVariable(VariablePtr(new Float(name, true, true, std::any_cast<ValuePtr>(ctx->constInitVal()->accept(this)))));
                // irModule.pushVariable(VariablePtr(new Float(name, true, true, ctx->constInitVal()->accept(this).as<ValuePtr>())));
        }
        else
        {
            auto curr = shared_ptr<ArrType>(new ArrType(irModule.declType, stoi(std::any_cast<ValuePtr>(ctx->children[childSize - 4]->accept(this))->getStr())));
            // auto curr = shared_ptr<ArrType>(new ArrType(irModule.declType, stoi(ctx->children[childSize - 4]->accept(this).as<ValuePtr>()->getStr())));
            
            for (int childInd = childSize - 7; childInd > 1; childInd -= 3)
            {
                curr = shared_ptr<ArrType>(new ArrType(curr, stoi(std::any_cast<ValuePtr>(ctx->children[childInd]->accept(this))->getStr())));
                // curr = shared_ptr<ArrType>(new ArrType(curr, stoi(ctx->children[childInd]->accept(this).as<ValuePtr>()->getStr())));
            }

            auto initVal = arrInitList(dynamic_cast<SysY2022Parser::ListConstInitValContext *>(ctx->constInitVal()), curr);
            initVal->name = name;
            irModule.pushVariable(initVal);
            //上面一行中已经包含该操作
            // irModule.currStringTable->insert(initVal);
        }
    }
    else // 局部变量
    {
        if (childSize == 3)
        {
            VariablePtr variable;
            if (irModule.declType->isInt())
                variable = VariablePtr(new Int(name, false, true, std::any_cast<ValuePtr>(ctx->constInitVal()->accept(this))));
                // variable = VariablePtr(new Int(name, false, true, ctx->constInitVal()->accept(this).as<ValuePtr>()));
            else if (irModule.declType->isFloat())
                variable = VariablePtr(new Float(name, false, true, std::any_cast<ValuePtr>(ctx->constInitVal()->accept(this))));
                // variable = VariablePtr(new Float(name, false, true, ctx->constInitVal()->accept(this).as<ValuePtr>()));
            irModule.currStringTable->insert(variable);
        }
        else // 数组
        {
            auto curr = shared_ptr<ArrType>(new ArrType(irModule.declType, stoi(std::any_cast<ValuePtr>(ctx->children[childSize - 4]->accept(this))->getStr())));
            // auto curr = shared_ptr<ArrType>(new ArrType(irModule.declType, stoi(ctx->children[childSize - 4]->accept(this).as<ValuePtr>()->getStr())));
            
            for (int childInd = childSize - 7; childInd > 1; childInd -= 3)
            {
                curr = shared_ptr<ArrType>(new ArrType(curr, stoi(std::any_cast<ValuePtr>(ctx->children[childInd]->accept(this))->getStr())));
                // curr = shared_ptr<ArrType>(new ArrType(curr, stoi(ctx->children[childInd]->accept(this).as<ValuePtr>()->getStr())));
            }

            auto initVal = arrInitList(dynamic_cast<SysY2022Parser::ListConstInitValContext *>(ctx->constInitVal()), curr);
            initVal->name = "__const." + to_string(constArr++);
            irModule.pushVariable(initVal);
            irModule.currStringTable->variableTable[name] = initVal;
        }
    }

    return nullptr;
}

antlrcpp::Any MyVisitor::visitAssignStmt(SysY2022Parser::AssignStmtContext *ctx)
{
    VariablePtr val = irModule.currStringTable->lookUp(ctx->lVal()->Identifier()->getText());
    if (val->isGlobal || val->type->isPtr())
        irModule.globalFunctions.back()->isReenterable = false;
    ValuePtr exp = std::any_cast<ValuePtr>(ctx->exp()->accept(this));
    // ValuePtr exp = ctx->exp()->accept(this).as<ValuePtr>();

    if (ctx->lVal()->children.size() == 1)
    {
        if (!val->type->operator==(exp->type))
        {
            if (val->type->isInt() && exp->type->isFloat())
            {
                // to-do: isconst
                auto ins = shared_ptr<FptosiInstruction>(new FptosiInstruction(exp, irModule.getBasicBlock()));
                irModule.getBasicBlock()->pushInstruction(ins);
                exp = ins->reg;
            }
            else if (val->type->isFloat() && exp->type->isInt())
            {
                // to-do: isconst
                auto ins = shared_ptr<SitofpInstruction>(new SitofpInstruction(exp, irModule.getBasicBlock()));
                irModule.getBasicBlock()->pushInstruction(ins);
                exp = ins->reg;
            }
        }
        irModule.getBasicBlock()->pushInstruction(shared_ptr<StoreInstruction>(new StoreInstruction(val, exp, irModule.getBasicBlock())));
    }
    else
    {
        ValuePtr curr = val;
        if (val->type->isPtr())
        {
            auto ins = shared_ptr<LoadInstruction>(new LoadInstruction(curr, irModule.getFunc()->getReg(dynamic_cast<PtrType *>(curr->type.get())->inner), irModule.getBasicBlock()));
            irModule.getBasicBlock()->pushInstruction(ins);
            curr = ins->to;

            // 解引用
            ValuePtr ind = std::any_cast<ValuePtr>(ctx->lVal()->children[2]->accept(this));
            // ValuePtr ind = ctx->lVal()->children[2]->accept(this).as<ValuePtr>();
            if (ind->type != Type::getInt64())
            {
                if (ind->isConst)
                {
                    // ind->type = Type::getInt64();
                    ind = Const::getConst(Type::getInt64(), dynamic_cast<Const*>(ind.get())->intVal,ind->name);
                }
                else
                {
                    auto sextIns = shared_ptr<ExtInstruction>(new ExtInstruction(ind, Type::getInt64(), true, irModule.getBasicBlock()));
                    irModule.getBasicBlock()->pushInstruction(sextIns);
                    ind = sextIns->reg;
                }
            }
            auto inst = shared_ptr<GetElementPtrInstruction>(new GetElementPtrInstruction(curr, vector<ValuePtr>({ind}), irModule.getBasicBlock()));
            irModule.getBasicBlock()->pushInstruction(inst);
            curr = inst->reg;

            // 处理数组
            if (ctx->lVal()->children.size() > 5)
            {
                vector<ValuePtr> indexs = {Const::getConst(Type::getInt64(), 0)};
                // vector<ValuePtr> indexs = {ValuePtr(new Const(Type::getInt64(), 0))};
                for (int i = 5; i < ctx->lVal()->children.size(); i += 3)
                {
                    ValuePtr ind = std::any_cast<ValuePtr>(ctx->lVal()->children[i]->accept(this));
                    // ValuePtr ind = ctx->lVal()->children[i]->accept(this).as<ValuePtr>();
                    if (ind->type != Type::getInt64())
                    {
                        if (ind->isConst)
                        {
                            // ind->type = Type::getInt64();
                            ind = Const::getConst(Type::getInt64(), dynamic_cast<Const*>(ind.get())->intVal,ind->name);
                        }
                        else
                        {
                            auto sextIns = shared_ptr<ExtInstruction>(new ExtInstruction(ind, Type::getInt64(), true, irModule.getBasicBlock()));
                            irModule.getBasicBlock()->pushInstruction(sextIns);
                            ind = sextIns->reg;
                        }
                    }
                    indexs.emplace_back(ind);
                }
                inst = shared_ptr<GetElementPtrInstruction>(new GetElementPtrInstruction(curr, indexs, irModule.getBasicBlock()));
                irModule.getBasicBlock()->pushInstruction(inst);
                curr = inst->reg;
            }
            if (!curr->type->operator==(exp->type))
                {
                    if(exp->isConst){
                        if(exp->type->isInt()){
                            // exp->type = Type::getFloat();
                            // dynamic_cast<Const *>(exp.get())->floatVal = dynamic_cast<Const *>(exp.get())->intVal;
                            exp = Const::getConst(Type::getFloat(),float(dynamic_cast<Const *>(exp.get())->intVal), exp->name);
                        }
                        else if(exp->type->isFloat()){
                            // exp->type = Type::getInt();
                            // dynamic_cast<Const *>(exp.get())->intVal = dynamic_cast<Const *>(exp.get())->floatVal;
                            exp = Const::getConst(Type::getInt(),int(dynamic_cast<Const *>(exp.get())->floatVal), exp->name);
                        }
                    }
                    else{
                        if (val->type->isInt() && exp->type->isFloat())
                        {
                            // to-do: isconst
                            auto ins = shared_ptr<FptosiInstruction>(new FptosiInstruction(exp, irModule.getBasicBlock()));
                            irModule.getBasicBlock()->pushInstruction(ins);
                            exp = ins->reg;
                        }
                        else if (val->type->isFloat() && exp->type->isInt())
                        {
                            // to-do: isconst
                            auto ins = shared_ptr<SitofpInstruction>(new SitofpInstruction(exp, irModule.getBasicBlock()));
                            irModule.getBasicBlock()->pushInstruction(ins);
                            exp = ins->reg;
                        }
                    }
                }
            irModule.getBasicBlock()->pushInstruction(shared_ptr<StoreInstruction>(new StoreInstruction(curr, exp, irModule.getBasicBlock())));
        }
        else
        {
            vector<ValuePtr> indexs = {Const::getConst(Type::getInt64(), 0)};
            // vector<ValuePtr> indexs = {ValuePtr(new Const(Type::getInt64(), 0))};
            for (int i = 2; i < ctx->lVal()->children.size(); i += 3)
            {
                ValuePtr ind = std::any_cast<ValuePtr>(ctx->lVal()->children[i]->accept(this));
                // ValuePtr ind = ctx->lVal()->children[i]->accept(this).as<ValuePtr>();
                if (ind->type != Type::getInt64())
                {
                    if (ind->isConst)
                    {
                        // ind->type = Type::getInt64();
                        ind = Const::getConst(Type::getInt64(), dynamic_cast<Const*>(ind.get())->intVal,ind->name);
                    }
                    else
                    {
                        auto sextIns = shared_ptr<ExtInstruction>(new ExtInstruction(ind, Type::getInt64(), true, irModule.getBasicBlock()));
                        irModule.getBasicBlock()->pushInstruction(sextIns);
                        ind = sextIns->reg;
                    }
                }
                indexs.emplace_back(ind);
            }

            auto ins = shared_ptr<GetElementPtrInstruction>(new GetElementPtrInstruction(curr, indexs, irModule.getBasicBlock()));
            curr = ins->reg;
            if (!curr->type->operator==(exp->type))
            {
                if (curr->type->isInt() && exp->type->isFloat())
                {
                    // to-do: isconst
                    auto ins = shared_ptr<FptosiInstruction>(new FptosiInstruction(exp, irModule.getBasicBlock()));
                    irModule.getBasicBlock()->pushInstruction(ins);
                    exp = ins->reg;
                }
                else if (curr->type->isFloat() && exp->type->isInt())
                {
                    // to-do: isconst
                    auto ins = shared_ptr<SitofpInstruction>(new SitofpInstruction(exp, irModule.getBasicBlock()));
                    irModule.getBasicBlock()->pushInstruction(ins);
                    exp = ins->reg;
                }
            }
            // if (val->isGlobal)
            // {
            //     bool isConst = true;
            //     for (int i = 0; i < indexs.size(); i++)
            //     {
            //         if (!indexs[i]->isConst)
            //         {
            //             isConst = false;
            //             break;
            //         }
            //     }
            //     if (isConst)
            //     {
            //         auto inst = shared_ptr<StoreInstruction>(new StoreInstruction(ins, exp, irModule.getBasicBlock()));
            //         irModule.getBasicBlock()->pushInstruction(inst);
            //         return exp; // 返回值
            //     }
            // }
            irModule.getBasicBlock()->pushInstruction(ins);
            irModule.getBasicBlock()->pushInstruction(shared_ptr<StoreInstruction>(new StoreInstruction(curr, exp, irModule.getBasicBlock())));
        }
    }
    return exp; // 返回值
}

antlrcpp::Any MyVisitor::visitExp(SysY2022Parser::ExpContext *ctx)
{
    return ctx->addExp()->accept(this);
}

antlrcpp::Any MyVisitor::visitLVal(SysY2022Parser::LValContext *ctx)
{
    VariablePtr val = irModule.currStringTable->lookUp(ctx->Identifier()->getText());

    int sz = ctx->children.size();

    if (sz > 1) // 数组
    {
        ValuePtr curr = val;
        vector<ValuePtr> indexs;
        for (int i = 2; i < sz; i += 3)
            indexs.emplace_back(std::any_cast<ValuePtr>(ctx->children[i]->accept(this)));
            // indexs.emplace_back(ctx->children[i]->accept(this).as<ValuePtr>());
        // cerr<<indexs.size()<<endl;
        if (val->type->isPtr()) // 函数参数访问
        {
            auto ins = shared_ptr<LoadInstruction>(new LoadInstruction(curr, irModule.getFunc()->getReg(dynamic_cast<PtrType *>(curr->type.get())->inner), irModule.getBasicBlock()));
            irModule.getBasicBlock()->pushInstruction(ins);
            curr = ins->to;

            // 解引用
            auto ind = indexs[0];
            if (ind->type != Type::getInt64())
            {
                if (ind->isConst)
                {
                    // ind->type = Type::getInt64();
                    ind = Const::getConst(Type::getInt64(), dynamic_cast<Const*>(ind.get())->intVal,ind->name);
                }
                else
                {
                    auto sextIns = shared_ptr<ExtInstruction>(new ExtInstruction(ind, Type::getInt64(), true, irModule.getBasicBlock()));
                    irModule.getBasicBlock()->pushInstruction(sextIns);
                    ind = sextIns->reg;
                }
            }
            auto inst = shared_ptr<GetElementPtrInstruction>(new GetElementPtrInstruction(curr, vector<ValuePtr>({ind}), irModule.getBasicBlock()));
            irModule.getBasicBlock()->pushInstruction(inst);
            curr = inst->reg;
            indexs.erase(indexs.begin());
            // 处理数组
            if (sz > 5)
            {
                // indexs.insert(indexs.begin(), ValuePtr(new Const(Type::getInt64(), 0)));
                indexs.insert(indexs.begin(), Const::getConst(Type::getInt64(), 0));
                for (int i = 1; i < indexs.size(); i++)
                {
                    auto ind = indexs[i];
                    if (ind->type != Type::getInt64())
                    {
                        if (ind->isConst)
                        {
                            // ind->type = Type::getInt64();
                            ind = Const::getConst(Type::getInt64(), dynamic_cast<Const*>(ind.get())->intVal,ind->name);
                        }
                        else
                        {
                            auto sextIns = shared_ptr<ExtInstruction>(new ExtInstruction(ind, Type::getInt64(), true, irModule.getBasicBlock()));
                            irModule.getBasicBlock()->pushInstruction(sextIns);
                            ind = sextIns->reg;
                        }
                        indexs[i] = ind;
                    }
                }
                inst = shared_ptr<GetElementPtrInstruction>(new GetElementPtrInstruction(curr, indexs, irModule.getBasicBlock()));
                irModule.getBasicBlock()->pushInstruction(inst);
                curr = inst->reg;
            }
            ins = shared_ptr<LoadInstruction>(new LoadInstruction(curr, irModule.getFunc()->getReg(curr->type), irModule.getBasicBlock()));
            irModule.getBasicBlock()->pushInstruction(ins);
            return ins->to;
        }
        else
        {
            if (val->isConst)
            {
                bool isConst = true;
                for (int i = 0; i < indexs.size(); i++)
                {
                    if (!indexs[i]->isConst)
                    {
                        isConst = false;
                        break;
                    }
                }
                if (isConst)
                {
                    for (int i = 0; i < indexs.size(); i++)
                    {
                        auto ind = dynamic_cast<Const *>(indexs[i].get())->intVal;
                        curr = dynamic_cast<Arr *>(curr.get())->inner[ind];
                    }
                    if (curr->type->isInt())
                        return Const::getConst(Type::getInt(),dynamic_cast<Int *>(curr.get())->intVal);
                        // return ValuePtr(new Const(dynamic_cast<Int *>(curr.get())->intVal));
                    else if (curr->type->isFloat())
                        return Const::getConst(Type::getFloat(),dynamic_cast<Float *>(curr.get())->floatVal);
                        // return ValuePtr(new Const(dynamic_cast<Float *>(curr.get())->floatVal));
                    else
                        assert(0);
                }
            }
            int depth = dynamic_cast<ArrType *>(val->type.get())->getDepth();
            // indexs.insert(indexs.begin(), ValuePtr(new Const(Type::getInt64(), 0)));
            indexs.insert(indexs.begin(), Const::getConst(Type::getInt64(), 0));
            // cerr<<indexs.size()<<endl;
            for (int i = 1; i < indexs.size(); i++)
            {
                auto ind = indexs[i];
                if (ind->type != Type::getInt64())
                {
                    if (ind->isConst)
                    {
                        // ind->type = Type::getInt64();
                        ind = Const::getConst(Type::getInt64(), dynamic_cast<Const*>(ind.get())->intVal,ind->name);
                    }
                    else
                    {
                        auto sextIns = shared_ptr<ExtInstruction>(new ExtInstruction(ind, Type::getInt64(), true, irModule.getBasicBlock()));
                        irModule.getBasicBlock()->pushInstruction(sextIns);
                        ind = sextIns->reg;
                    }
                    indexs[i] = ind;
                }
            }
            if (sz == depth * 3 + 1) // 访问元素
            {
                shared_ptr<LoadInstruction> inst;
                // if (val->isGlobal)
                // {
                //     bool isConst = true;
                //     for (int i = 0; i < indexs.size(); i++)
                //     {
                //         if (!indexs[i]->isConst)
                //         {
                //             isConst = false;
                //             break;
                //         }
                //     }
                //     if (isConst)
                //     {
                //         auto ins = shared_ptr<GetElementPtrInstruction>(new GetElementPtrInstruction(val, indexs, irModule.getBasicBlock()));
                //         inst = shared_ptr<LoadInstruction>(new LoadInstruction(ins, irModule.getFunc()->getReg(ins->reg->type), irModule.getBasicBlock()));
                //         irModule.getBasicBlock()->pushInstruction(inst);
                //         return inst->to;
                //     }
                // }
                auto ins = shared_ptr<GetElementPtrInstruction>(new GetElementPtrInstruction(val, indexs, irModule.getBasicBlock()));
                irModule.getBasicBlock()->pushInstruction(ins);
                inst = shared_ptr<LoadInstruction>(new LoadInstruction(ins->reg, irModule.getFunc()->getReg(ins->reg->type), irModule.getBasicBlock()));
                irModule.getBasicBlock()->pushInstruction(inst);
                return inst->to;
            }
            else // 访问行指针
            {
                //bug :30 and 39测例
                // for (int i = sz + 1; i < depth * 3 + 1; i += 3)
                // {
                //     indexs.emplace_back(ValuePtr(new Const(Type::getInt64(), 0)));
                // }
                // cerr<<indexs.size()<<endl;
                // indexs.emplace_back(ValuePtr(new Const(Type::getInt64(), 0)));
                indexs.emplace_back(Const::getConst(Type::getInt64(), 0));
                auto ins = shared_ptr<GetElementPtrInstruction>(new GetElementPtrInstruction(curr, indexs, irModule.getBasicBlock()));
                ins->reg->type = TypePtr(new PtrType(ins->reg->type)); // 指针化
                // ins->reg->type  = TypePtr(new PtrType(dynamic_cast<ArrType *>(ins->reg->type.get())->inner));
                irModule.getBasicBlock()->pushInstruction(ins);
                return ins->reg;
            }
        }
    }
    else
    {
        if (val->isConst)
        {
            if (val->type->isInt())
                return Const::getConst(Type::getInt(),dynamic_cast<Int *>(val.get())->intVal);
                // return ValuePtr(new Const(dynamic_cast<Int *>(val.get())->intVal));
            else
                return Const::getConst(Type::getFloat(), dynamic_cast<Float *>(val.get())->floatVal);
                // return ValuePtr(new Const(Type::getFloat(), dynamic_cast<Float *>(val.get())->floatVal));
        }
        else
        {
            if (val->type->isArr()) // maybe bug
            {
                // vector<ValuePtr> index = {
                //     ValuePtr(new Const(Type::getInt64(), 0)),
                //     ValuePtr(new Const(Type::getInt64(), 0))};
                vector<ValuePtr> index = {
                    Const::getConst(Type::getInt64(), 0),
                    Const::getConst(Type::getInt64(), 0)};
                auto ins = shared_ptr<GetElementPtrInstruction>(new GetElementPtrInstruction(val, index, irModule.getBasicBlock()));
                irModule.getBasicBlock()->pushInstruction(ins);
                ins->reg->type = TypePtr(new PtrType(ins->reg->type));
                return ins->reg;
            }
            else
            {
                auto ins = shared_ptr<LoadInstruction>(new LoadInstruction(val, irModule.getFunc()->getReg(val->type), irModule.getBasicBlock()));
                irModule.getBasicBlock()->pushInstruction(ins);
                return ins->to;
            }
        }
    }
}

antlrcpp::Any MyVisitor::visitExpPrimaryExp(SysY2022Parser::ExpPrimaryExpContext *ctx)
{
    return ctx->exp()->accept(this);
}

antlrcpp::Any MyVisitor::visitLValPrimaryExp(SysY2022Parser::LValPrimaryExpContext *ctx)
{
    return ctx->lVal()->accept(this);
}

antlrcpp::Any MyVisitor::visitNumberPrimaryExp(SysY2022Parser::NumberPrimaryExpContext *ctx)
{
    return ctx->number()->accept(this);
}

antlrcpp::Any MyVisitor::visitNumber(SysY2022Parser::NumberContext *ctx)
{
    if (ctx->getStart()->getType() == SysY2022Lexer::IntConst){
        return Const::getConst(Type::getInt(), ctx->getText());
    }
    else
        return Const::getConst(Type::getFloat(), ctx->getText());
}

antlrcpp::Any MyVisitor::visitPrimaryUnaryExp(SysY2022Parser::PrimaryUnaryExpContext *ctx)
{
    auto primaryExp = std::any_cast<ValuePtr>(ctx->primaryExp()->accept(this));
    // auto primaryExp = ctx->primaryExp()->accept(this).as<ValuePtr>();
    // 处理 int 溢出
    if (primaryExp->isConst && primaryExp->type->isInt() && dynamic_cast<Const *>(primaryExp.get())->intVal == 0x80000000)
        dynamic_cast<Const *>(primaryExp.get())->intVal = (int)0x80000000;
    return primaryExp;
}

antlrcpp::Any MyVisitor::visitFunctionCallUnaryExp(SysY2022Parser::FunctionCallUnaryExpContext *ctx)
{
    vector<ValuePtr> argv;
    if (ctx->children.size() == 4)
        argv = std::any_cast<vector<ValuePtr>>(ctx->funcRParams()->accept(this));
        // argv = ctx->funcRParams()->accept(this).as<vector<ValuePtr>>();
    auto func = irModule.getFunction(ctx->Identifier()->getText());
    if (!func->isReenterable)
        irModule.globalFunctions.back()->isReenterable = false;
    if (func->name.substr(0, 6) == "_sysy_")
    {
        // argv.emplace_back(ValuePtr(new Const(Type::getInt(), 0)));
        argv.emplace_back(Const::getConst(Type::getInt(), 0));
    }
    for (int i = 0; i < argv.size(); i++)
    {
        if (!argv[i]->type->operator==(func->formArguments[i]->type))
        {
            if (argv[i]->isConst)
            {
                if (argv[i]->type->isFloat() && func->formArguments[i]->type->isInt())
                {
                    // argv[i]->type = Type::getInt();
                    // dynamic_cast<Const *>(argv[i].get())->intVal = dynamic_cast<Const *>(argv[i].get())->floatVal;
                    argv[i] = Const::getConst(Type::getInt(),int(dynamic_cast<Const *>(argv[i].get())->floatVal), argv[i]->name);
                }
                else if(argv[i]->type->isInt() && func->formArguments[i]->type->isFloat()){
                    // argv[i]->type = Type::getFloat();
                    // dynamic_cast<Const *>(argv[i].get())->floatVal = dynamic_cast<Const *>(argv[i].get())->intVal;
                    argv[i] = Const::getConst(Type::getFloat(),float(dynamic_cast<Const *>(argv[i].get())->intVal), argv[i]->name);
                }
                // to-do
            }
            else
            {
                if (argv[i]->type->isFloat() && func->formArguments[i]->type->isInt())
                {
                    auto ins = shared_ptr<FptosiInstruction>(new FptosiInstruction(argv[i], irModule.getBasicBlock()));
                    irModule.getBasicBlock()->pushInstruction(ins);
                    argv[i] = ins->reg;
                }
                else if (argv[i]->type->isInt() && func->formArguments[i]->type->isFloat())
                {
                    auto ins = shared_ptr<SitofpInstruction>(new SitofpInstruction(argv[i], irModule.getBasicBlock()));
                    irModule.getBasicBlock()->pushInstruction(ins);
                    argv[i] = ins->reg;
                }
                //to-do 打个补丁先，问题来源于gep最后给出的是arr类型而非ptr类型
                // else if (argv[i]->type->isArr()){
                //     argv[i]->type = TypePtr(new PtrType(dynamic_cast<ArrType *>(argv[i]->type.get())->inner));
                // }
                else
                {
                    auto ins = shared_ptr<BitCastInstruction>(new BitCastInstruction(argv[i], irModule.getFunc()->getReg(func->formArguments[i]->type), irModule.getBasicBlock(), func->formArguments[i]->type));
                    irModule.getBasicBlock()->pushInstruction(ins);
                    argv[i] = ins->reg;
                }
            }
        }
    }
    auto ins = shared_ptr<CallInstruction>(new CallInstruction(func, argv, irModule.getBasicBlock()));
    irModule.getBasicBlock()->pushInstruction(ins);
    return ins->reg;
}

antlrcpp::Any MyVisitor::visitFuncRParams(SysY2022Parser::FuncRParamsContext *ctx)
{
    vector<ValuePtr> rst;
    for (int i = 0; i < ctx->children.size(); i += 2)
    {
        rst.emplace_back(std::any_cast<ValuePtr>(ctx->children[i]->accept(this)));
        // rst.emplace_back(ctx->children[i]->accept(this).as<ValuePtr>());
    }
    return rst;
}

antlrcpp::Any MyVisitor::visitUnaryUnaryExp(SysY2022Parser::UnaryUnaryExpContext *ctx)
{
    char op = ctx->unaryOp()->getText()[0];
    auto exp = std::any_cast<ValuePtr>(ctx->unaryExp()->accept(this));
    // auto exp = ctx->unaryExp()->accept(this).as<ValuePtr>();

    switch (op)
    {
    case '-':
        if (exp->isConst)
        {
            if (exp->type->isInt()){
                // dynamic_cast<Const *>(exp.get())->intVal *= -1;
                exp = Const::getConst(Type::getInt(),-1*dynamic_cast<Const*>(exp.get())->intVal,exp->name);
            }
            else if (exp->type->isFloat()){
                // dynamic_cast<Const *>(exp.get())->floatVal *= -1;
                exp = Const::getConst(Type::getFloat(),float(-1.0*dynamic_cast<Const*>(exp.get())->floatVal),exp->name);
            }
        }
        else
        {
            if (exp->type->isBool())
            {
                auto ins = shared_ptr<ExtInstruction>(new ExtInstruction(exp, Type::getInt(), false, irModule.getBasicBlock()));
                irModule.getBasicBlock()->pushInstruction(InstructionPtr(ins));
                exp = ins->reg;
            }
            if (exp->type->isInt())
            {
                // auto ins = shared_ptr<BinaryInstruction>(new BinaryInstruction(ValuePtr(new Const(0)), exp, op, irModule.getBasicBlock()));
                auto ins = shared_ptr<BinaryInstruction>(new BinaryInstruction(Const::getConst(Type::getInt(),0), exp, op, irModule.getBasicBlock()));
                irModule.getBasicBlock()->pushInstruction(InstructionPtr(ins));
                exp = ins->reg;
            }
            else if (exp->type->isFloat())
            {
                auto ins = shared_ptr<FnegInstruction>(new FnegInstruction(exp, irModule.getBasicBlock()));
                irModule.getBasicBlock()->pushInstruction(ins);
                exp = ins->reg;
            }
        }
        break;
    case '+':
        // 处理 int 溢出
        if (exp->isConst && exp->type->isInt() && dynamic_cast<Const *>(exp.get())->intVal == 0x80000000)
            dynamic_cast<Const *>(exp.get())->intVal = (int)0x80000000;
        if (exp->type->isBool())
        {
            auto ins = shared_ptr<ExtInstruction>(new ExtInstruction(exp, Type::getInt(), false, irModule.getBasicBlock()));
            irModule.getBasicBlock()->pushInstruction(InstructionPtr(ins));
            exp = ins->reg;
        }
        break;
    case '!':
        if (!exp->type->isBool())
        {
            if (exp->type->isFloat())
            {
                auto instruction = shared_ptr<FcmpInstruction>(new FcmpInstruction(irModule.getBasicBlock(), exp));
                irModule.getBasicBlock()->pushInstruction(instruction);
                exp = instruction->reg;
            }
            else
            {
                auto instruction = shared_ptr<IcmpInstruction>(new IcmpInstruction(irModule.getBasicBlock(), exp));
                irModule.getBasicBlock()->pushInstruction(instruction);
                exp = instruction->reg;
            }
        };
        if (exp->isConst)
        {
            if (exp->type->isInt()){
                // dynamic_cast<Const *>(exp.get())->boolVal = !dynamic_cast<Const *>(exp.get())->intVal;
                exp = Const::getConst(Type::getBool(),bool(!dynamic_cast<Const*>(exp.get())->intVal),exp->name);
            }
            else if (exp->type->isFloat()){
                // dynamic_cast<Const *>(exp.get())->boolVal = std::abs(dynamic_cast<Const *>(exp.get())->floatVal) < 1e-6;
                exp = Const::getConst(Type::getBool(),std::abs(dynamic_cast<Const *>(exp.get())->floatVal) < 1e-6,exp->name);
            }
            // dynamic_cast<Const *>(exp.get())->type = Type::getBool();
            // exp = Const::getConst(Type::getBool(),bool(dynamic_cast<Const*>(exp.get())->boolVal),exp->name);
        }
        else
        {
            // auto ins = shared_ptr<BinaryInstruction>(new BinaryInstruction(exp, ValuePtr(new Const(true)), op, irModule.getBasicBlock()));
            auto ins = shared_ptr<BinaryInstruction>(new BinaryInstruction(exp, Const::getConst(Type::getBool(),true), op, irModule.getBasicBlock()));
            // ins->print();
            irModule.getBasicBlock()->pushInstruction(InstructionPtr(ins));
            exp = ins->reg;
        }
    default:
        break;
    }

    return exp;
}

antlrcpp::Any MyVisitor::visitMulMulExp(SysY2022Parser::MulMulExpContext *ctx)
{
    char op = ctx->children[1]->getText()[0];
    auto a = std::any_cast<ValuePtr>(ctx->mulExp()->accept(this));
    // auto a = ctx->mulExp()->accept(this).as<ValuePtr>();
    auto b = std::any_cast<ValuePtr>(ctx->unaryExp()->accept(this));
    // auto b = ctx->unaryExp()->accept(this).as<ValuePtr>();

    if (a->isConst && b->isConst)
    {
        assert(!a->type->isBool() && !b->type->isBool());
        if (a->type->isFloat() || b->type->isFloat())
        {
            float valA = a->type->isFloat() ? (dynamic_cast<Const *>(a.get())->floatVal) : (dynamic_cast<Const *>(a.get())->intVal);
            float valB = b->type->isFloat() ? (dynamic_cast<Const *>(b.get())->floatVal) : (dynamic_cast<Const *>(b.get())->intVal);
            float rst = 0;
            if (op == '*')
                rst = valA * valB;
            else if (op == '/')
                rst = valA / valB;
            // return ValuePtr(new Const(Type::getFloat(), rst));
            return Const::getConst(Type::getFloat(), rst);
        }
        else
        {
            auto valA = dynamic_cast<Const *>(a.get())->intVal;
            auto valB = dynamic_cast<Const *>(b.get())->intVal;
            long long rst = 0;
            if (op == '*')
                rst = valA * valB;
            else if (op == '/')
                rst = valA / valB;
            else
                rst = valA % valB;
            // return ValuePtr(new Const(Type::getInt(), (int)rst));
            return Const::getConst(Type::getInt(), (int)rst);
        }
    }
    if (a->type->isFloat() || b->type->isFloat())
    {
        if (!a->type->isFloat())
        {
            assert(a->type->isInt());
            if (a->isConst)
            {
                // auto constA = dynamic_cast<Const *>(a.get());
                // a->type = Type::getFloat();
                // constA->floatVal = constA->intVal;
                a = Const::getConst(Type::getFloat(),float(dynamic_cast<Const *>(a.get())->intVal), a->name);
            }
            else
            {
                auto instruction = shared_ptr<SitofpInstruction>(new SitofpInstruction(a, irModule.getBasicBlock()));
                irModule.getBasicBlock()->pushInstruction(instruction);
                a = instruction->reg;
            }
        }
        else if (!b->type->isFloat())
        {
            assert(b->type->isInt());
            if (b->isConst)
            {
                // auto constB = dynamic_cast<Const *>(b.get());
                // b->type = Type::getFloat();
                // constB->floatVal = constB->intVal;
                b = Const::getConst(Type::getFloat(),float(dynamic_cast<Const *>(b.get())->intVal), b->name);
            }
            else
            {
                auto instruction = shared_ptr<SitofpInstruction>(new SitofpInstruction(b, irModule.getBasicBlock()));
                irModule.getBasicBlock()->pushInstruction(instruction);
                b = instruction->reg;
            }
        }
    }
    auto instruction = shared_ptr<BinaryInstruction>(new BinaryInstruction(a, b, op, irModule.getBasicBlock()));
    irModule.getBasicBlock()->pushInstruction(instruction);
    return instruction->reg;
}

antlrcpp::Any MyVisitor::visitUnaryMulExp(SysY2022Parser::UnaryMulExpContext *ctx)
{
    return ctx->unaryExp()->accept(this);
}

antlrcpp::Any MyVisitor::visitAddAddExp(SysY2022Parser::AddAddExpContext *ctx)
{
    char op = ctx->children[1]->getText()[0];
    auto a = std::any_cast<ValuePtr>(ctx->addExp()->accept(this));
    // auto a = ctx->addExp()->accept(this).as<ValuePtr>();
    auto b = std::any_cast<ValuePtr>(ctx->mulExp()->accept(this));
    // auto b = ctx->mulExp()->accept(this).as<ValuePtr>();
    if (a->isConst && b->isConst)
    {
        assert(!a->type->isBool() && !b->type->isBool());
        if (a->type->isFloat() || b->type->isFloat())
        {
            float valA = a->type->isFloat() ? (dynamic_cast<Const *>(a.get())->floatVal) : (dynamic_cast<Const *>(a.get())->intVal);
            float valB = b->type->isFloat() ? (dynamic_cast<Const *>(b.get())->floatVal) : (dynamic_cast<Const *>(b.get())->intVal);
            float rst = 0;
            if (op == '+')
                rst = valA + valB;
            else if (op == '-')
                rst = valA - valB;
            // return ValuePtr(new Const(Type::getFloat(), rst));
            return Const::getConst(Type::getFloat(), rst);
        }
        else
        {
            auto valA = dynamic_cast<Const *>(a.get())->intVal;
            auto valB = dynamic_cast<Const *>(b.get())->intVal;
            long long rst = 0;
            if (op == '+')
                rst = valA + valB;
            else if (op == '-')
                rst = valA - valB;
            // return ValuePtr(new Const(Type::getInt(), (int)rst));
            return Const::getConst(Type::getInt(), (int)rst);
        }
    }
    if (a->type->isFloat() || b->type->isFloat())
    {
        if (!a->type->isFloat())
        {
            assert(a->type->isInt());
            if (a->isConst)
            {
                // auto constA = dynamic_cast<Const *>(a.get());
                // a->type = Type::getFloat();
                // constA->floatVal = constA->intVal;
                a = Const::getConst(Type::getFloat(),float(dynamic_cast<Const *>(a.get())->intVal), a->name);
                
            }
            else
            {
                auto instruction = shared_ptr<SitofpInstruction>(new SitofpInstruction(a, irModule.getBasicBlock()));
                irModule.getBasicBlock()->pushInstruction(instruction);
                a = instruction->reg;
            }
        }
        else if (!b->type->isFloat())
        {
            assert(b->type->isInt());
            if (b->isConst)
            {
                // auto constB = dynamic_cast<Const *>(b.get());
                // b->type = Type::getFloat();
                // constB->floatVal = constB->intVal;
                b = Const::getConst(Type::getFloat(),float(dynamic_cast<Const *>(b.get())->intVal), b->name);
            }
            else
            {
                auto instruction = shared_ptr<SitofpInstruction>(new SitofpInstruction(b, irModule.getBasicBlock()));
                irModule.getBasicBlock()->pushInstruction(instruction);
                b = instruction->reg;
            }
        }
    }
    auto instruction = shared_ptr<BinaryInstruction>(new BinaryInstruction(a, b, op, irModule.getBasicBlock()));
    irModule.getBasicBlock()->pushInstruction(instruction);

    return instruction->reg;
}

antlrcpp::Any MyVisitor::visitMulAddExp(SysY2022Parser::MulAddExpContext *ctx)
{
    return ctx->mulExp()->accept(this);
}

antlrcpp::Any MyVisitor::visitConstExp(SysY2022Parser::ConstExpContext *ctx)
{
    return ctx->addExp()->accept(this);
}

antlrcpp::Any MyVisitor::visitIfStmt(SysY2022Parser::IfStmtContext *ctx)
{
    auto trueLabel = LabelPtr(new Label(BrInstruction::getifThenStr()));
    auto falseLabel = LabelPtr(new Label(BrInstruction::getifEndStr()));
    irModule.trueLabelStack.emplace(trueLabel);
    irModule.falseLabelStack.emplace(falseLabel);
    auto cond = ctx->cond()->accept(this);
    irModule.trueLabelStack.pop();
    irModule.falseLabelStack.pop();

    irModule.getFunc()->pushBasicBlock(BasicBlockPtr(new BasicBlock(trueLabel)));
    ctx->stmt()->accept(this);
    irModule.getBasicBlock()->setEndInstruction(InstructionPtr(new BrInstruction(falseLabel, irModule.getBasicBlock())));
    irModule.getFunc()->pushBasicBlock(BasicBlockPtr(new BasicBlock(falseLabel)));
    return nullptr;
}

antlrcpp::Any MyVisitor::visitIfElseStmt(SysY2022Parser::IfElseStmtContext *ctx)
{
    auto trueLabel = LabelPtr(new Label(BrInstruction::getifThenStr()));
    auto falseLabel = LabelPtr(new Label(BrInstruction::getifElseStr()));
    auto endLabel = LabelPtr(new Label(BrInstruction::getifEndStr()));
    irModule.trueLabelStack.emplace(trueLabel);
    irModule.falseLabelStack.emplace(falseLabel);
    ctx->cond()->accept(this);
    irModule.trueLabelStack.pop();
    irModule.falseLabelStack.pop();

    irModule.getFunc()->pushBasicBlock(BasicBlockPtr(new BasicBlock(trueLabel)));
    ctx->children[4]->accept(this);
    irModule.getBasicBlock()->setEndInstruction(InstructionPtr(new BrInstruction(endLabel, irModule.getBasicBlock())));
    irModule.getFunc()->pushBasicBlock(BasicBlockPtr(new BasicBlock(falseLabel)));
    ctx->children[6]->accept(this);
    irModule.getBasicBlock()->setEndInstruction(InstructionPtr(new BrInstruction(endLabel, irModule.getBasicBlock())));
    irModule.getFunc()->pushBasicBlock(BasicBlockPtr(new BasicBlock(endLabel)));

    return nullptr;
}

antlrcpp::Any MyVisitor::visitWhileStmt(SysY2022Parser::WhileStmtContext *ctx)
{
    auto trueLabel = LabelPtr(new Label(BrInstruction::getwhileBodyStr()));
    auto falseLabel = LabelPtr(new Label(BrInstruction::getwhileEndStr()));
    auto condLabel = LabelPtr(new Label(BrInstruction::getWhileCondStr()));

    irModule.getBasicBlock()->setEndInstruction(InstructionPtr(new BrInstruction(condLabel, irModule.getBasicBlock())));
    irModule.getFunc()->pushBasicBlock(BasicBlockPtr(new BasicBlock(condLabel)));
    irModule.whileCondStack.emplace(condLabel);
    irModule.whileEndStack.emplace(falseLabel);
    irModule.trueLabelStack.emplace(trueLabel);
    irModule.falseLabelStack.emplace(falseLabel);
    ctx->cond()->accept(this);
    irModule.trueLabelStack.pop();
    irModule.falseLabelStack.pop();
    irModule.getFunc()->pushBasicBlock(BasicBlockPtr(new BasicBlock(trueLabel)));
    ctx->stmt()->accept(this);
    irModule.getBasicBlock()->setEndInstruction(InstructionPtr(new BrInstruction(condLabel, irModule.getBasicBlock())));
    irModule.getFunc()->pushBasicBlock(BasicBlockPtr(new BasicBlock(falseLabel)));
    irModule.whileEndStack.pop();
    irModule.whileCondStack.pop();

    return nullptr;
}

antlrcpp::Any MyVisitor::visitBreakStmt(SysY2022Parser::BreakStmtContext *ctx)
{
    irModule.getBasicBlock()->setEndInstruction(InstructionPtr(new BrInstruction(irModule.whileEndStack.top(), irModule.getBasicBlock())));
    return nullptr;
}

antlrcpp::Any MyVisitor::visitContinueStmt(SysY2022Parser::ContinueStmtContext *ctx)
{
    irModule.getBasicBlock()->setEndInstruction(InstructionPtr(new BrInstruction(irModule.whileCondStack.top(), irModule.getBasicBlock())));
    return nullptr;
}

antlrcpp::Any MyVisitor::visitRelRelExp(SysY2022Parser::RelRelExpContext *ctx)
{
    string op = ctx->children[1]->getText();
    auto a = std::any_cast<ValuePtr>(ctx->relExp()->accept(this));
    // auto a = ctx->relExp()->accept(this).as<ValuePtr>();
    auto b = std::any_cast<ValuePtr>(ctx->addExp()->accept(this));
    // auto b = ctx->addExp()->accept(this).as<ValuePtr>();
    if (a->isConst && b->isConst)
    {
        auto constA = dynamic_cast<Const *>(a.get());
        auto constB = dynamic_cast<Const *>(b.get());
        long long valA = constA->type->isBool() ? constA->boolVal : constA->intVal;
        long long valB = constB->type->isBool() ? constB->boolVal : constB->intVal;
        if (op == ">")
        {
            // return ValuePtr(new Const(valA > valB));
            return Const::getConst(Type::getBool(),valA > valB);
        }
        else if (op == ">=")
        {
            // return ValuePtr(new Const(valA >= valB));
            return Const::getConst(Type::getBool(),valA >= valB);
        }
        else if (op == "<")
        {
            // return ValuePtr(new Const(valA < valB));
            return Const::getConst(Type::getBool(),valA < valB);
        }
        else
        {
            // return ValuePtr(new Const(valA <= valB));
            return Const::getConst(Type::getBool(),valA <= valB);
        }
    }
    else
    {
        if (a->type->isFloat() || b->type->isFloat())
        {
            if (!b->type->isFloat())
            {
                assert(b->type->isInt());
                if (b->isConst)
                {
                    // auto constB = dynamic_cast<Const *>(b.get());
                    // constB->type = a->type;
                    // constB->floatVal = constB->intVal;
                    b = Const::getConst(a->type,float(dynamic_cast<Const *>(b.get())->intVal), b->name);
                }
            }
            auto ins = shared_ptr<FcmpInstruction>(new FcmpInstruction(irModule.getBasicBlock(), a, b, op));
            irModule.getBasicBlock()->pushInstruction(ins);
            auto trueLabel = irModule.trueLabelStack.top();
            auto falseLabel = irModule.falseLabelStack.top();
            return ins->reg;
        }
        else
        {
            auto ins = shared_ptr<IcmpInstruction>(new IcmpInstruction(irModule.getBasicBlock(), a, b, op));
            irModule.getBasicBlock()->pushInstruction(ins);
            auto trueLabel = irModule.trueLabelStack.top();
            auto falseLabel = irModule.falseLabelStack.top();
            return ins->reg;
        }
    }
}

antlrcpp::Any MyVisitor::visitAddRelExp(SysY2022Parser::AddRelExpContext *ctx)
{
    return visitChildren(ctx);
}

antlrcpp::Any MyVisitor::visitEqEqExp(SysY2022Parser::EqEqExpContext *ctx)
{
    string op = ctx->children[1]->getText();
    auto a = std::any_cast<ValuePtr>(ctx->eqExp()->accept(this));
    // auto a = ctx->eqExp()->accept(this).as<ValuePtr>();
    auto b = std::any_cast<ValuePtr>(ctx->relExp()->accept(this));
    // auto b = ctx->relExp()->accept(this).as<ValuePtr>();
    if (a->isConst && b->isConst)
    {
        auto constA = dynamic_cast<Const *>(a.get());
        auto constB = dynamic_cast<Const *>(b.get());
        long long valA = constA->type->isBool() ? constA->boolVal : constA->intVal;
        long long valB = constB->type->isBool() ? constB->boolVal : constB->intVal;
        if (op == "==")
        {
            // return ValuePtr(new Const(valA == valB));
            return Const::getConst(Type::getBool(),valA == valB);
        }
        else
        {
            // return ValuePtr(new Const(valA != valB));
            return Const::getConst(Type::getBool(),valA != valB);
        }
    }
    else
    {
        if (!a->type->operator==(b->type))
        {

            if (a->type->isBool())
            {
                if(b->type->isInt()){
                    auto instruction = shared_ptr<ExtInstruction>(new ExtInstruction(a, Type::getInt(), false, irModule.getBasicBlock()));
                    irModule.getBasicBlock()->pushInstruction(instruction);
                    a = instruction->reg;
                    auto ins = shared_ptr<IcmpInstruction>(new IcmpInstruction(irModule.getBasicBlock(), a, b, op));
                    irModule.getBasicBlock()->pushInstruction(InstructionPtr(ins));
                    auto trueLabel = irModule.trueLabelStack.top();
                    auto falseLabel = irModule.falseLabelStack.top();
                    return ins->reg;
                }
                //float
                else{
                    auto instruction = shared_ptr<ExtInstruction>(new ExtInstruction(a, Type::getInt(), false, irModule.getBasicBlock()));
                    irModule.getBasicBlock()->pushInstruction(instruction);
                    a = instruction->reg;
                    auto instruction2 = shared_ptr<SitofpInstruction>(new SitofpInstruction(a, irModule.getBasicBlock()));
                    irModule.getBasicBlock()->pushInstruction(instruction2);
                    a = instruction2->reg;
                    auto ins = shared_ptr<FcmpInstruction>(new FcmpInstruction(irModule.getBasicBlock(), a, b, op));
                    irModule.getBasicBlock()->pushInstruction(InstructionPtr(ins));
                    auto trueLabel = irModule.trueLabelStack.top();
                    auto falseLabel = irModule.falseLabelStack.top();
                    return ins->reg;
                }
            }
            else if(b->type->isBool()){
                if(a->type->isInt()){
                    auto instruction = shared_ptr<ExtInstruction>(new ExtInstruction(b, Type::getInt(), false, irModule.getBasicBlock()));
                    irModule.getBasicBlock()->pushInstruction(instruction);
                    b = instruction->reg;
                    auto ins = shared_ptr<IcmpInstruction>(new IcmpInstruction(irModule.getBasicBlock(), a, b, op));
                    irModule.getBasicBlock()->pushInstruction(InstructionPtr(ins));
                    auto trueLabel = irModule.trueLabelStack.top();
                    auto falseLabel = irModule.falseLabelStack.top();
                    return ins->reg;
                }
                else{
                    auto instruction = shared_ptr<ExtInstruction>(new ExtInstruction(b, Type::getInt(), false, irModule.getBasicBlock()));
                    irModule.getBasicBlock()->pushInstruction(instruction);
                    b = instruction->reg;
                    auto instruction2 = shared_ptr<SitofpInstruction>(new SitofpInstruction(b, irModule.getBasicBlock()));
                    irModule.getBasicBlock()->pushInstruction(instruction2);
                    b = instruction2->reg;
                    auto ins = shared_ptr<FcmpInstruction>(new FcmpInstruction(irModule.getBasicBlock(), a, b, op));
                    irModule.getBasicBlock()->pushInstruction(InstructionPtr(ins));
                    auto trueLabel = irModule.trueLabelStack.top();
                    auto falseLabel = irModule.falseLabelStack.top();
                    return ins->reg;
                }
            }
            //to-do  这里特判了一种情况
            else if(a->type->isFloat()||b->type->isFloat()){
                if(a->type->isFloat()){
                    if(b->isConst){
                        // b->type = Type::getFloat();
                        // dynamic_cast<Const *>(b.get())->floatVal = dynamic_cast<Const *>(b.get())->intVal;
                        b = Const::getConst(Type::getFloat(),float(dynamic_cast<Const *>(b.get())->intVal), b->name);
                        auto ins = shared_ptr<FcmpInstruction>(new FcmpInstruction(irModule.getBasicBlock(), a, b, op));
                        irModule.getBasicBlock()->pushInstruction(InstructionPtr(ins));
                        auto trueLabel = irModule.trueLabelStack.top();
                        auto falseLabel = irModule.falseLabelStack.top();
                        return ins->reg;
                    }  
                    else{
                        //to-do
                    }
                }
                else{
                    if(a->isConst){
                        // a->type = Type::getFloat();
                        // dynamic_cast<Const *>(a.get())->floatVal = dynamic_cast<Const *>(a.get())->intVal;
                        a = Const::getConst(Type::getFloat(),float(dynamic_cast<Const *>(a.get())->intVal), a->name);
                        auto ins = shared_ptr<FcmpInstruction>(new FcmpInstruction(irModule.getBasicBlock(), a, b, op));
                        irModule.getBasicBlock()->pushInstruction(InstructionPtr(ins));
                        auto trueLabel = irModule.trueLabelStack.top();
                        auto falseLabel = irModule.falseLabelStack.top();
                        return ins->reg;
                    }
                    else{

                    }
                }
            }
            else
            {
                auto instruction = shared_ptr<ExtInstruction>(new ExtInstruction(b, Type::getInt(), false, irModule.getBasicBlock()));
                irModule.getBasicBlock()->pushInstruction(instruction);
                b = instruction->reg;
                auto ins = shared_ptr<IcmpInstruction>(new IcmpInstruction(irModule.getBasicBlock(), a, b, op));
                irModule.getBasicBlock()->pushInstruction(InstructionPtr(ins));
                auto trueLabel = irModule.trueLabelStack.top();
                auto falseLabel = irModule.falseLabelStack.top();
                return ins->reg;
            }
        }
        //无特殊情况
        auto ins = shared_ptr<IcmpInstruction>(new IcmpInstruction(irModule.getBasicBlock(), a, b, op));
        irModule.getBasicBlock()->pushInstruction(InstructionPtr(ins));
        auto trueLabel = irModule.trueLabelStack.top();
        auto falseLabel = irModule.falseLabelStack.top();
        return ins->reg;
    }
}

antlrcpp::Any MyVisitor::visitRelEqExp(SysY2022Parser::RelEqExpContext *ctx)
{
    return visitChildren(ctx);
}

antlrcpp::Any MyVisitor::visitEqLAndExp(SysY2022Parser::EqLAndExpContext *ctx)
{
    auto trueLabel = irModule.trueLabelStack.top();
    auto falseLabel = irModule.falseLabelStack.top();

    auto val = std::any_cast<ValuePtr>(ctx->eqExp()->accept(this));
    // auto val = ctx->eqExp()->accept(this).as<ValuePtr>();

    if (!val->type->isBool())
    {
        if (val->type->isFloat())
        {
            auto instruction = shared_ptr<FcmpInstruction>(new FcmpInstruction(irModule.getBasicBlock(), val));
            irModule.getBasicBlock()->pushInstruction(instruction);
            val = instruction->reg;
        }
        else
        {
            auto instruction = shared_ptr<IcmpInstruction>(new IcmpInstruction(irModule.getBasicBlock(), val));
            irModule.getBasicBlock()->pushInstruction(instruction);
            val = instruction->reg;
        }
    }
    irModule.getBasicBlock()->setEndInstruction(InstructionPtr((new BrInstruction(val, trueLabel, falseLabel, irModule.getBasicBlock()))));
    if (val->isConst)
        return val;
    else
        return nullptr;
}

antlrcpp::Any MyVisitor::visitLAndLAndExp(SysY2022Parser::LAndLAndExpContext *ctx)
{
    
    auto trueLabel = irModule.trueLabelStack.top();
    auto falseLabel = irModule.falseLabelStack.top();
    auto andLabel = LabelPtr(new Label(BrInstruction::getAndStr()));
    irModule.trueLabelStack.emplace(andLabel);
    auto lAnd = ctx->lAndExp()->accept(this);
    irModule.trueLabelStack.pop();

    irModule.getFunc()->pushBasicBlock(BasicBlockPtr(new BasicBlock(andLabel)));
    auto val = std::any_cast<ValuePtr>(ctx->eqExp()->accept(this));
    // auto val = ctx->eqExp()->accept(this).as<ValuePtr>();
    if (!val->type->isBool())
    {
        auto instruction = shared_ptr<IcmpInstruction>(new IcmpInstruction(irModule.getBasicBlock(), val));
        irModule.getBasicBlock()->pushInstruction(instruction);
        val = instruction->reg;
    }
    irModule.getBasicBlock()->setEndInstruction(InstructionPtr((new BrInstruction(val, trueLabel, falseLabel, irModule.getBasicBlock()))));

    if (lAnd.has_value()&& lAnd.type() != typeid(nullptr)&& val->isConst)
    {
        auto land = std::any_cast<ValuePtr>(lAnd);
        // auto land = lAnd.as<ValuePtr>();
        bool val1 = dynamic_cast<Const *>(land.get())->boolVal;
        bool val2 = dynamic_cast<Const *>(val.get())->boolVal;
        bool rst = val1 && val2;
        irModule.getFunc()->basicBlocks.pop_back();
        if (rst)
        {
            irModule.getBasicBlock()->endInstruction = InstructionPtr(new BrInstruction(trueLabel, irModule.getBasicBlock()));
        }
        else
        {
            irModule.getBasicBlock()->endInstruction = InstructionPtr(new BrInstruction(falseLabel, irModule.getBasicBlock()));
        }
        // return ValuePtr(new Const(Type::getBool(), rst));
        return Const::getConst(Type::getBool(), rst);
    }
    return nullptr;
}

antlrcpp::Any MyVisitor::visitLAndLOrExp(SysY2022Parser::LAndLOrExpContext *ctx)
{
    return ctx->lAndExp()->accept(this);
}

antlrcpp::Any MyVisitor::visitLOrLOrExp(SysY2022Parser::LOrLOrExpContext *ctx)
{
    // cerr<<ctx->toStringTree(true)<<endl;
    auto trueLabel = irModule.trueLabelStack.top();
    auto falseLabel = irModule.falseLabelStack.top();
    auto orLabel = LabelPtr(new Label(BrInstruction::getOrStr()));
    irModule.falseLabelStack.emplace(orLabel);
    auto lOr = ctx->lOrExp()->accept(this);
    irModule.falseLabelStack.pop();
    irModule.getFunc()->pushBasicBlock(BasicBlockPtr(new BasicBlock(orLabel)));
    auto lAnd = ctx->lAndExp()->accept(this);
    if (lAnd.has_value()&& lAnd.type() != typeid(nullptr) && lOr.has_value()&& lOr.type() == typeid(nullptr))
    {
        bool val1=false,val2=false;
        // cerr<<lAnd.type().name()<<endl;
        // cerr<<lOr.type().name()<<endl;
        try{
            val1 = dynamic_cast<Const *>(std::any_cast<ValuePtr>(lAnd).get())->boolVal;
            // bool val1 = dynamic_cast<Const *>(lAnd.as<ValuePtr>().get())->boolVal;
        }
        catch(const std::bad_any_cast& e) {
            std::cerr<<"And Bad any cast caught : "<<e.what()<<endl;
        }
        try{
            val2 = dynamic_cast<Const *>(std::any_cast<ValuePtr>(lOr).get())->boolVal;
            // bool val2 = dynamic_cast<Const *>(lOr.as<ValuePtr>().get())->boolVal;
        }
        catch(const std::bad_any_cast& e) {
            std::cerr<<"Or Bad any cast caught : "<<e.what()<<endl;
        }
        bool rst = val1 || val2;
        irModule.getFunc()->basicBlocks.pop_back();
        if (rst)
        {
            irModule.getBasicBlock()->endInstruction = InstructionPtr(new BrInstruction(trueLabel, irModule.getBasicBlock()));
        }
        else
        {
            irModule.getBasicBlock()->endInstruction = InstructionPtr(new BrInstruction(falseLabel, irModule.getBasicBlock()));
        }
        // return ValuePtr(new Const(Type::getBool(), rst));
        return Const::getConst(Type::getBool(), rst);
    }
    return nullptr;
}