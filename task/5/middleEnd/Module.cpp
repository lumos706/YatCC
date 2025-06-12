#include "Module.h"

Module::Module()
{
    globalStringTable = StringTableNodePtr(new StringTableNode());
    currStringTable = globalStringTable;

    vector<ValuePtr> memsetArgv = {
        RegPtr(new Reg(TypePtr(new PtrType(Type::getInt8())), "")),
        RegPtr(new Reg(Type::getInt8(), "")),
        RegPtr(new Reg(Type::getInt64(), "")),
        RegPtr(new Reg(Type::getBool(), ""))};
    globalFunctions.emplace_back(FunctionPtr(new Function(Type::getVoid(), "llvm.memset.p0i8.i64", true, memsetArgv)));
    
    vector<ValuePtr> memcpyArgv = {
        RegPtr(new Reg(TypePtr(new PtrType(Type::getInt8())), "")),
        RegPtr(new Reg(TypePtr(new PtrType(Type::getInt8())), "")),
        RegPtr(new Reg(Type::getInt64(), "")),
        RegPtr(new Reg(Type::getBool(), ""))};
    globalFunctions.emplace_back(FunctionPtr(new Function(Type::getVoid(), "llvm.memcpy.p0i8.p0i8.i64", true, memcpyArgv)));
    
    vector<ValuePtr> putintArgv = {
        RegPtr(new Reg(Type::getInt(), ""))};
    globalFunctions.emplace_back(FunctionPtr(new Function(Type::getVoid(), "putint", false, putintArgv)));
    
    globalFunctions.emplace_back(FunctionPtr(new Function(Type::getVoid(), "putch", false, putintArgv)));
    
    vector<ValuePtr> putfloatArgv = {
        RegPtr(new Reg(Type::getFloat(), ""))};
    globalFunctions.emplace_back(FunctionPtr(new Function(Type::getVoid(), "putfloat", false, putfloatArgv)));
    
    vector<ValuePtr> putArrayArgv = {
        RegPtr(new Reg(Type::getInt(), "")),
        RegPtr(new Reg(TypePtr(new PtrType(Type::getInt())), ""))};
    globalFunctions.emplace_back(FunctionPtr(new Function(Type::getVoid(), "putarray", false, putArrayArgv)));
    vector<ValuePtr> putfArrayArgv = {
        RegPtr(new Reg(Type::getInt(), "")),
        RegPtr(new Reg(TypePtr(new PtrType(Type::getFloat())), ""))};
    globalFunctions.emplace_back(FunctionPtr(new Function(Type::getVoid(), "putfarray", false, putfArrayArgv)));

    globalFunctions.emplace_back(FunctionPtr(new Function(Type::getInt(), "getint", false)));
    globalFunctions.emplace_back(FunctionPtr(new Function(Type::getInt(), "getch", false)));
    globalFunctions.emplace_back(FunctionPtr(new Function(Type::getFloat(), "getfloat", false)));
    vector<ValuePtr> getArrayArgv = {
        RegPtr(new Reg(TypePtr(new PtrType(Type::getInt())), ""))};
    globalFunctions.emplace_back(FunctionPtr(new Function(Type::getInt(), "getarray", false, getArrayArgv)));
    vector<ValuePtr> getfArrayArgv = {
        RegPtr(new Reg(TypePtr(new PtrType(Type::getFloat())), ""))};
    globalFunctions.emplace_back(FunctionPtr(new Function(Type::getInt(), "getfarray", false, getfArrayArgv)));
    vector<ValuePtr> timeArgv = {
        RegPtr(new Reg(Type::getInt(), ""))};

    globalFunctions.emplace_back(FunctionPtr(new Function(Type::getVoid(), "_sysy_starttime", false, timeArgv)));
    globalFunctions.emplace_back(FunctionPtr(new Function(Type::getVoid(), "_sysy_stoptime", false, timeArgv)));
}

void Module::pushGlobalFunction(FunctionPtr Function) { globalFunctions.emplace_back(Function); }

void Module::pushVariable(VariablePtr globalVariable)
{
    //to-do 
    globalVariables.emplace_back(globalVariable);
    globalStringTable->insert(globalVariables.back());
}

void Module::pushFunc(FunctionPtr func)
{
    funcStack.emplace(func);
}

FunctionPtr Module::popFunc()
{
    FunctionPtr tmp = funcStack.top();
    funcStack.pop();
    return tmp;
}

StringTableNodePtr Module::popStringTable()
{
    StringTableNodePtr tmp = currStringTable;
    currStringTable = currStringTable->father;
    return tmp;
}

FunctionPtr Module::getFunc()
{
    return funcStack.top();
}

BasicBlockPtr Module::getBasicBlock()
{
    return funcStack.top()->basicBlocks.back();
}

FunctionPtr Module::getFunction(string name)
{
    if(name=="starttime") return globalFunctions[12]; 
    if(name=="stoptime") return globalFunctions[13]; 
    for (auto func : globalFunctions)
    {
        if (name == func->name)
            return func;
    }
    std::cerr << "func " << name << " not find!" << endl;
    return nullptr;
}

bool Module::blockStackEmpty()
{
    return funcStack.empty();
}

void Module::pushBackStringTable()
{
    currStringTable = StringTableNodePtr(new StringTableNode(currStringTable));
}

void Module::registerVariable(VariablePtr variable)
{
    //fix :局部alloca的变量修正为Ptr,因为是通过指针来访问这些变量的
    //将原本的变量类型套上一层ptr
    // TypePtr addPtr = TypePtr(new PtrType(variable->type));
    // variable = VariablePtr(new Ptr(variable->name,variable->isGlobal,variable->isConst,addPtr));


    VariablePtr tmp = currStringTable->lookUp(variable->name);

    int stringLen = variable->name.length();
    if (stringLen > 1024)
    {
        string newName = variable->name.substr(0, 1024);
        currStringTable->variableTable[variable->name] = variable;
        variable->name = newName;
    }
    //处理同名，alloca统一在entry开头进行，不能有同名
    else if (tmp && !(tmp->isGlobal) || getFunc()->findVariable(variable->name))
    {
        int suffix = 0;
        string oldName = variable->name;
        string newName = oldName;
        do
        {
            newName = oldName + to_string(suffix++); // to-do: 使用更高效的办法
        } while (currStringTable->lookUp(newName) || getFunc()->findVariable(newName));
        variable->name += to_string(suffix - 1);
        currStringTable->variableTable[oldName] = variable;
    }
    else
    {
        currStringTable->insert(variable);
    }
    getFunc()->pushVariable(variable);
}

void Module::print()
{
    for (auto &i : globalVariables)
        i->print();
    for (auto &i : globalFunctions)
        if (!i->isLib)
            i->print();
    for (auto &i : globalFunctions)
        if (i->isLib)
            i->print();
}