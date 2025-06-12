#pragma once

#include <vector>
#include <cassert>
#include <cmath>
#include <cinttypes>
#include <iostream>
#include <unordered_map>
#include "Type.h"

using std::unordered_map;
using std::vector;
using namespace std;

struct Value;
struct Instruction;


//每个Value的use类组织成一个双向链表，删除时间复杂度为O(1)
struct Value;
struct Instruction;

// 很重要的数据结构，用于记录value的调用情况
struct Use {
    // 双向链表
    Use *prev = nullptr, *next = nullptr;
    // 调用的value
    Value *val;
    // 调用的指令
    Instruction *user;
    // 指令的value，我们没有很好的和llvm一样统一value和ins
    Value *userVal;
    // 是否无人使用
    bool isDead = false;
    // 删除自身
    void rmUse();
    void useDead() {
        isDead = true; 
    }
};

struct Value
{
    // 变量类型
    TypePtr type;
    // 变量名
    string name;
    // 是否是常量
    bool isConst=false;
    // 是否是寄存器
    bool isReg=false;
    // 是否是void
    bool isVoid=false;
    Value(TypePtr type, string name, bool isConst, bool isReg, bool isVoid) : type{type}, name{name}, isConst{isConst}, isReg{isReg}, isVoid{isVoid} {}
    virtual string getStr(){return "";}
    // 输出llvm ir相关的函数
    virtual string myTypeStr() { return type->getStr(); };
    virtual string getTypeStr() { return type->getStr() + " " + getStr(); };
    virtual string getPointStr() { return type->getStr() + "* " + getStr(); };

    // 每一个变量都会维护一个自己的use链表
    Use* useHead=nullptr; 
    // 记录该value被使用的次数
    int numUses = 0;
    int getNumUses() { return numUses; }
    
    //记录该reg所代表的指令
    Instruction *I = nullptr;
    // 添加use到链表
    void addUse(Use * u){
        u->prev = nullptr;
        if(useHead){
            useHead->prev = u;
        }
        u->next = useHead;
        useHead = u;
        numUses ++;
    }

    bool hasOneUse(){
        return useHead&&!(useHead->next); 
    }
};
typedef shared_ptr<Value> ValuePtr;
// 创建与查找use类
Use *newUse(Value *value, Instruction *user);
Use *newUse(Value *value, Instruction *user, Value *userVal);
Use *findUse(Value *value, Value *userVal);
Use *findUse(Value *value, Instruction *user);



struct Reg : Value
{
    static ValuePtr getReg(TypePtr type, int id) { return ValuePtr(new Reg(type, id)); }
    //暂时加个reg，不然跑不了
    Reg(TypePtr type, int id) : Value{type, "reg" + to_string(id), false, true, false} {}
    Reg(TypePtr type, string name) : Value{type, name, false, true, false} {}
    virtual string getStr() override { return "%" + name; }
};
typedef shared_ptr<Reg> RegPtr;


//常量使用单例模式来管理会更好
struct Const : Value
{
    //单例模式
    static unordered_map<int,ValuePtr> IntConstMap;
    static unordered_map<long long,ValuePtr> longlongConstMap;
    static unordered_map<float, ValuePtr> FloatConstMap;
    static unordered_map<bool, ValuePtr> BoolConstMap;
    static unordered_map<int8_t, ValuePtr> Int8ConstMap;
    // 提前准备好各种类型的常量，按照类型取值
    long long intVal;     
    float floatVal;
    bool boolVal;
    // 创建各种类型常量的函数
    Const(bool boolVal, string name = "") : Value{Type::getBool(), name, true, false, false}, boolVal{boolVal} {};
    Const(int intVal, string name = "") : Value{Type::getInt(), name, true, false, false}, intVal{intVal} {};
    Const(long long intVal, string name = "") : Value{Type::getInt(), name, true, false, false}, intVal{intVal} {};
    Const(float floatVal, string name = "") : Value{Type::getFloat(), name, true, false, false}, floatVal{floatVal} {};
    Const(TypePtr type, bool boolVal, string name = "") : Value{type, name, true, false, false}, boolVal{boolVal} {};
    Const(TypePtr type, int intVal, string name = "") : Value{type, name, true, false, false}, intVal{intVal} {};
    Const(TypePtr type, long long intVal, string name = "") : Value{type, name, true, false, false}, intVal{intVal} {};
    Const(TypePtr type, float floatVal, string name = "") : Value{type, name, true, false, false}, floatVal{floatVal} {};
    Const(TypePtr type, string value, string name = "") : Value{type, name, true, false, false}
    {
        try
        {
            if (type->isFloat())
            {
                floatVal = std::stod(value);
            }
            else
            {
                int scale = 10;
                if (value.size() > 2 && value.substr(0, 2) == "0x")
                    scale = 16;
                else if (value.size() > 1 && value[0] == '0')
                    scale = 8;
                intVal = stoll(value, 0, scale);
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "const init error(" << intVal << ", " << floatVal << ")" << '\n';
        }
    };

    
    virtual string getStr() override;
    public:
    static ValuePtr getConst(TypePtr type,int val,string name = "");
    static ValuePtr getConst(TypePtr type,int8_t val,string name = "");
    static ValuePtr getConst(TypePtr type,float val,string name = "");
    static ValuePtr getConst(TypePtr type,bool val,string name = "");
    static ValuePtr getConst(TypePtr type,string val,string name = "");
    static ValuePtr getConst(TypePtr type,long long val,string name = "");

};




// 各种类型变量

struct Void : Value
{
    static ValuePtr value;
    static ValuePtr get() { return value; }
    Void() : Value{Type::getVoid(), "", false, false, true} {};
    virtual string getStr() override { return ""; };
};

struct Variable : Value
{
    // 是否是全局变量
    bool isGlobal;
    Variable(TypePtr type, string name, bool isGlobal, bool isConst) : Value{type, name, isConst, false, false}, isGlobal{isGlobal} {};
    virtual string getStr() override;

    virtual void print() = 0;
    virtual void printHelper() = 0;
    virtual bool zero() = 0;
    static shared_ptr<Variable> copy(shared_ptr<Variable> var);
};
typedef shared_ptr<Variable> VariablePtr;

struct Int : Variable
{
    // int值
    long long intVal;
    Int(string name, bool isGlobal, bool isConst) : Variable{Type::getInt(), name, isGlobal, isConst}, intVal{0} {};
    Int(string name, bool isGlobal, bool isConst, long long intVal) : Variable{Type::getInt(), name, isGlobal, isConst}, intVal{intVal} {};
    Int(string name, bool isGlobal, bool isConst, ValuePtr value);
    // 输出llvm ir相关的函数
    virtual void print() override;
    virtual void printHelper() override;
    virtual bool zero() override;
};

struct Float : Variable
{
    float floatVal;
    Float(string name, bool isGlobal, bool isConst) : Variable{Type::getFloat(), name, isGlobal, isConst}, floatVal{0} {};
    Float(string name, bool isGlobal, bool isConst, ValuePtr value);
    Float(string name, bool isGlobal, bool isConst, float value) : Variable{Type::getFloat(), name, isGlobal, isConst}, floatVal{value} {};
    virtual void print() override;
    virtual void printHelper() override;
    virtual bool zero() override;
};

struct Arr : Variable
{
    // 初始化的数据
    vector<VariablePtr> inner;
    Arr(string name, bool isGlobal, bool isConst, TypePtr type) : Variable{type, name, isGlobal, isConst} {};
    TypePtr getElementType() { return dynamic_cast<ArrType *>(type.get())->inner; }
    int getElementLength() { return dynamic_cast<ArrType *>(type.get())->length; }

    bool push(VariablePtr Variable);
    void fill();

    virtual void print() override;
    virtual void printHelper() override;
    virtual bool zero() override;
};

struct Ptr : Variable
{
    Ptr(string name, bool isGlobal, bool isConst, TypePtr type) : Variable{type, name, isGlobal, isConst} {};
    virtual void print() override;
    virtual void printHelper() override;
    virtual bool zero() override;
};

bool rmInstructionUse(shared_ptr<Instruction> I,ValuePtr v);
bool rmInstructionUse(Instruction* I,ValuePtr v);