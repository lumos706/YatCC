#include "constReplace.h"
#include <unordered_map>

using std::unordered_map;

// 测例优化空间有限， const仅传播一次
void globalConstReplace(Module &ir)
{
    unordered_map<Variable *, bool> isWrote;
    for (auto &func : ir.globalFunctions)
    {
        if (func->isLib)
            continue;
        for (auto &bb : func->basicBlocks)
        {
            for (auto &ins : bb->instructions)
            {
                if (ins->type == InsID::Store)
                {
                    auto storeIns = dynamic_cast<StoreInstruction *>(ins.get());
                    if (!storeIns->des->isReg)
                    {
                        auto var = dynamic_cast<Variable *>(storeIns->des.get());
                        isWrote[var] = true;
                    }
                    if (storeIns->gep)
                    {
                        auto gepIns = dynamic_cast<GetElementPtrInstruction *>(storeIns->gep.get());
                        if (!gepIns->from->isReg && gepIns->from->type->isArr())
                        {
                            auto arr = dynamic_cast<Arr *>(gepIns->from.get());
                            if (arr->isGlobal)
                            {
                                isWrote[arr] = true;
                            }
                        }
                    }
                }
                else if (ins->type == InsID::GEP)
                {
                    auto gepIns = dynamic_cast<GetElementPtrInstruction *>(ins.get());
                    if (!gepIns->from->isReg && gepIns->from->type->isArr())
                    {
                        auto arr = dynamic_cast<Arr *>(gepIns->from.get());
                        if (arr->isGlobal)
                        {
                            Use *p = gepIns->reg->useHead;
                            while (p != nullptr)
                            {
                                if (p->user->type == InsID::Call)
                                {
                                    isWrote[arr] = true;
                                    break;
                                }
                                else if (p->user->type == InsID::Store)
                                {
                                    isWrote[arr] = true;
                                    break;
                                }
                                else if (p->user->type == InsID::GEP)
                                {
                                    isWrote[arr] = true;
                                    break;
                                }
                                p = p->next;
                            }
                        }
                    }
                }
            }
        }
    }

    for (auto &var : ir.globalVariables)
    {
        if (var->isConst || isWrote[var.get()])
            continue;
        else if (var->type->isArr())
        {
            dynamic_cast<Arr*>(var.get())->fill();
            Use *p = var->useHead;
            while (p != nullptr)
            {
                auto user = p->user;
                if (user->type == InsID::Load)
                {
                    auto loadIns = dynamic_cast<LoadInstruction *>(user);
                    auto reg = loadIns->to;
                    if (loadIns->gep)
                    {
                        auto gepIns = loadIns->gep;
                        auto indexs = gepIns->index;

                        auto curr = var;
                        for (int i = 1; i < indexs.size(); i++)
                        {
                            auto ind = dynamic_cast<Const *>(indexs[i].get())->intVal;
                            curr = dynamic_cast<Arr *>(curr.get())->inner[ind];
                        }
                        ValuePtr constVal;
                        if (curr->type->isInt())
                            constVal = Const::getConst(Type::getInt(),dynamic_cast<Int *>(curr.get())->intVal);
                            // constVal = ValuePtr(new Const(dynamic_cast<Int *>(curr.get())->intVal));
                        else if (curr->type->isFloat())
                            constVal = Const::getConst(Type::getFloat(),dynamic_cast<Float *>(curr.get())->floatVal);
                            // constVal = ValuePtr(new Const(dynamic_cast<Float *>(curr.get())->floatVal));
                        replaceVarByVar(reg, constVal);
                        user->deleteSelfInBB();
                        // cerr<<var->getTypeStr()<<endl;
                    }
                }
                auto tem = p->next;
                p->rmUse();
                p = tem;
            }
        }
        else
        {
            ValuePtr constVal;
            if (var->type->isInt())
                // constVal = ValuePtr(new Const(dynamic_cast<Int *>(var.get())->intVal));
                constVal = Const::getConst(Type::getInt(),dynamic_cast<Int *>(var.get())->intVal);
            else
            {
                assert(var->type->isFloat());
                // constVal = ValuePtr(new Const(dynamic_cast<Float *>(var.get())->floatVal));
                constVal = Const::getConst(Type::getFloat(),dynamic_cast<Float *>(var.get())->floatVal);
            }

            Use *p = var->useHead;
            while (p != nullptr)
            {
                auto user = p->user;
                if (user->type == InsID::Load)
                {
                    auto loadIns = dynamic_cast<LoadInstruction *>(user);
                    auto reg = loadIns->to;
                    //replaceVarByVar remove use
                    replaceVarByVar(reg, constVal);
                    // cerr<<var->getTypeStr()<<endl;
                    user->deleteSelfInBB();
                }
                else
                {
                    cerr << "not load global replace" << endl;
                    //replaceValue donot remove use
                    user->replaceValue(var, constVal);
                }
                auto tem = p->next;
            
                p->rmUse();
                p = tem;
            }
        }
    }
}