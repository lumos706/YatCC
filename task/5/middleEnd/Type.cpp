#include "Type.h"

vector<shared_ptr<Type>> Type::types = {
    TypePtr(new Type(VoidID)),
    TypePtr(new Type(BoolID)),
    TypePtr(new Type(IntID)),
    TypePtr(new Type(FLoatID)),
    TypePtr(new Type(Int8ID)),
    TypePtr(new Type(Int64ID))};

Type::Type(string type)
{
    if (type == "int")
    {
        ID = IntID;
    }
    else if (type == "void")
    {
        ID = VoidID;
    }
    else if (type == "bool")
    {
        ID = BoolID;
    }
    else if (type == "float")
    {
        ID = FLoatID;
    }
    else if (type == "int8")
    {
        ID = Int8ID;
    }
    else if (type == "int64")
    {
        ID = Int64ID;
    }
    else
    {
        cout << "wrong type: " << type << endl;
    }
}

string Type::getStr()
{
    if (ID == IntID)
    {
        return "i32";
    }
    else if (ID == VoidID)
    {
        return "void";
    }
    else if (ID == BoolID)
    {
        return "i1";
    }
    else if (ID == FLoatID)
    {
        return "float";
    }
    else if (ID == Int8ID)
    {
        return "i8";
    }
    else if (ID == Int64ID)
    {
        return "i64";
    }
    else
    {
        return "wrong type";
    }
}

string ArrType::getStr()
{
    return "[" + to_string(length) + " x " + inner->getStr() + "]";
}

int ArrType::getSize()
{
    if (inner->isArr())
        return length * (dynamic_cast<ArrType *>(inner.get())->getSize());
    else
        return length;
}

int ArrType::getDepth()
{
    if(inner->isArr()) return 1 + dynamic_cast<ArrType*>(inner.get())->getDepth();
    else return 1;
}

bool ArrType::operator==(shared_ptr<Type> ptr)
{
    if (ptr->isArr())
    {
        auto arrPtr = dynamic_cast<ArrType *>(ptr.get());
        return arrPtr->length == length && inner->operator==(arrPtr->inner); // 这里可能有问题
    }
    else
        return false;
}

string PtrType::getStr()
{
    return inner->getStr() + "*";
}

bool PtrType::operator==(shared_ptr<Type> ptr)
{
    if (ptr->isPtr())
    {
        auto another = dynamic_cast<PtrType *>(ptr.get());
        return inner->operator==(another->inner); // 这里可能有问题
    }
    else
        return false;
}
