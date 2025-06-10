#pragma once
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <memory>

using std::cout;
using std::endl;
using std::shared_ptr;
using std::string;
using std::to_string;
using std::vector;

// 所有变量类型
enum TypeID
{
    VoidID,
    BoolID,
    IntID,
    FLoatID,
    Int8ID,
    Int64ID,
    ArrID,
    PtrID
};

struct Type
{
    // 变量类型
    TypeID ID;

    Type(TypeID type = VoidID) : ID{type} {};
    Type(string type);

    // 简单的返回信息函数
    int getID() { return (int)ID; }
    virtual string getStr();
    virtual bool operator==(shared_ptr<Type> ptr) { return ID == ptr->ID; }
    bool isVoid() { return ID == VoidID; }
    bool isBool() { return ID == BoolID; }
    bool isInt() { return ID == IntID || ID == Int8ID || ID == Int64ID; }
    bool isFloat() { return ID == FLoatID; }
    bool isArr() { return ID == ArrID; }
    bool isPtr() { return ID == PtrID; }

    // 方便我们进行变量类型设定
    static vector<shared_ptr<Type>> types;
    static shared_ptr<Type> getVoid() { return Type::types[0]; }
    static shared_ptr<Type> getBool() { return Type::types[1]; }
    static shared_ptr<Type> getInt() { return Type::types[2]; }
    static shared_ptr<Type> getFloat() { return Type::types[3]; }
    static shared_ptr<Type> getInt8() { return Type::types[4]; }
    static shared_ptr<Type> getInt64() { return Type::types[5]; }
};
typedef shared_ptr<Type> TypePtr;

struct ArrType : Type
{
    // 数组内包含的元素类型
    TypePtr inner;
    // 数组长度
    int length;
    ArrType(TypePtr inner, int length) : Type{ArrID}, inner{inner}, length{length} {};
    virtual string getStr() override;
    // inner类型以及数组长度一样为true
    virtual bool operator==(shared_ptr<Type> ptr) override;
    int getSize();
    int getDepth();
};

struct PtrType : Type
{
    // 指针指向的数据类型type
    TypePtr inner;
    PtrType(TypePtr inner) : Type{PtrID}, inner{inner} {}
    virtual string getStr() override;
    // inner类型一样为true
    virtual bool operator==(shared_ptr<Type> ptr) override;
};
