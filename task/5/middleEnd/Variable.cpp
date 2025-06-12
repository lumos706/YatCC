#include "Variable.h"

ValuePtr Void::value = ValuePtr(new Void());

unordered_map<int,ValuePtr> Const::IntConstMap;
unordered_map<float, ValuePtr> Const::FloatConstMap;
unordered_map<bool, ValuePtr> Const::BoolConstMap;
unordered_map<long long, ValuePtr> Const::longlongConstMap;
unordered_map<int8_t, ValuePtr> Const::Int8ConstMap;


ValuePtr Const::getConst(TypePtr type,int val,string name){
    if(type == Type::getFloat()){
        return getConst(type,float(val),name);
    }
    if(type == Type::getBool()){
        return getConst(type,bool(val),name);
    }
    if(type == Type::getInt8()){
        return getConst(type,int8_t(val),name);
    }
    if(type == Type::getInt64()){
        return getConst(type,(long long)(val),name);
    }
    if(IntConstMap.find(val)!=IntConstMap.end()){
        return IntConstMap[val];
    }
    ValuePtr ret = ValuePtr(new Const(type, val, name));
    return IntConstMap[val] = ret;
}
ValuePtr Const::getConst(TypePtr type,int8_t val,string name){
    if(type == Type::getFloat()){
        return getConst(type,float(val),name);
    }
    if(type == Type::getBool()){
        return getConst(type,bool(val),name);
    }
    if(type == Type::getInt()){
        return getConst(type,int(val),name);
    }
    if(type == Type::getInt64()){
        return getConst(type,(long long)(val),name);
    }
    if(Int8ConstMap.find(val)!=Int8ConstMap.end()){
        return Int8ConstMap[val];
    }
    ValuePtr ret = ValuePtr(new Const(type, val, name));
    return Int8ConstMap[val] = ret;
}
ValuePtr Const::getConst(TypePtr type,float val,string name){
    if(type==Type::getInt()||type==Type::getInt64()||type==Type::getInt8()){
        return getConst(type,int(val),name);
    }
    if(type == Type::getBool()){
        return getConst(type,bool(val),name);
    }
    if(FloatConstMap.find(val)!=FloatConstMap.end()){
        return FloatConstMap[val];
    }
    ValuePtr ret = ValuePtr(new Const(type, val, name));
    return FloatConstMap[val] = ret;
}
ValuePtr Const::getConst(TypePtr type,bool val,string name){
    if(type==Type::getInt()||type==Type::getInt64()||type==Type::getInt8()){
        return getConst(type,int(val),name);
    }
    if(type == Type::getFloat()){
        return getConst(type,float(val),name);
    }
    // std::cerr<<val<<std::endl;
    if(BoolConstMap.find(val)!=BoolConstMap.end()){
        return BoolConstMap[val];
    }
    ValuePtr ret = ValuePtr(new Const(type, val, name));
    // std::cerr<<ret->getTypeStr()<<std::endl;
    return BoolConstMap[val] = ret;
}

ValuePtr Const::getConst(TypePtr type,long long val,string name){
    if(type == Type::getFloat()){
        return getConst(type,float(val),name);
    }
    if(type == Type::getBool()){
        return getConst(type,bool(val),name);
    }
    if(type == Type::getInt8()){
        return getConst(type,int8_t(val),name);
    }
    if(type == Type::getInt()){
        return getConst(type,(int)(val),name);
    }
    if(longlongConstMap.find(val)!=longlongConstMap.end()){
        return longlongConstMap[val];
    }
    ValuePtr ret = ValuePtr(new Const(type, val, name));
    return longlongConstMap[val] = ret;
}

ValuePtr Const::getConst(TypePtr type,string val,string name){
    float floatVal;
    long long intVal;
    try
    {
        if (type->isFloat())
        {
            floatVal = std::stod(val);
            return getConst(type,floatVal,name);
        }
        else
        {
            int scale = 10;
            if (val.size() > 2 && val.substr(0, 2) == "0x")
                scale = 16;
            else if (val.size() > 1 && val[0] == '0')
                scale = 8;
            intVal = stoll(val, 0, scale);
            return getConst(type, intVal, name);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "const init error(" << intVal << ", " << floatVal << ")" << '\n';
        return nullptr;
    }
}

Use *newUse(Value *value, Instruction *user) {
    auto use = new Use;
    use->val = value;
    use->user = user;
    use->userVal = nullptr;
    value->addUse(use);
    return use;
}

Use *newUse(Value *value, Instruction *user, Value *userVal) {
    auto use = new Use;
    use->val = value;
    use->user = user;
    use->userVal = userVal;
    value->addUse(use);
    return use;
}

Use *findUse(Value *value, Value *userVal) {
    Use *use = value->useHead, *next = nullptr;
    while(use)
    {
        next = use->next;
        if (use->userVal == userVal) return use;
        use = next;
    }
    return nullptr;
}

Use *findUse(Value *value, Instruction *user) {
    Use *use = value->useHead, *next = nullptr;
    while(use)
    {
        next = use->next;
        if (use->user == user) return use;
        use = next;
    }
    return nullptr;
}

void Use::rmUse() {
    if (prev == nullptr && next == nullptr) {
        val->useHead = nullptr;
    } 
    else if (prev == nullptr && next != nullptr) {
        val->useHead = next;
        next->prev = nullptr;
    } 
    else if (prev != nullptr && next == nullptr) {
        prev->next = nullptr;
    } 
    else if (prev != nullptr && next != nullptr) {
        prev->next = next;
        next->prev = prev;
    }
    else{
        assert(false&&"rmUse error\n");
    }
    this->val->numUses --;
}

bool rmInstructionUse(shared_ptr<Instruction> I,ValuePtr v){
    auto useH = v->useHead;
    bool flag = false;
    while(useH!=nullptr){
        if(useH->user == I.get()){
            useH->rmUse();
            // return true;
            flag = true;
        }
        useH=useH->next;
    }
    return flag;
}

//非智能指针版本
bool rmInstructionUse(Instruction* I,ValuePtr v){
    auto useH = v->useHead;
    bool flag = false;
    while(useH!=nullptr){
        if(useH->user == I){
            useH->rmUse();
            // return true;
            flag = true;
        }
        useH=useH->next;
    }
    return flag;
}



string Const::getStr()
{
    if (type->isInt())
        return to_string(intVal);
    else if (type->isBool())
        return boolVal ? "true" : "false";
    else if (type->isFloat())
    {
        union
        {
            double f;
            uint64_t u;
        } f2u;
        f2u.f = floatVal;
        char buff[20] = {};
        sprintf(buff, "0x%" PRIx64, f2u.u);

        return string(buff);
    }
    else
        return "wrong const type!";
}

string Variable::getStr()
{
    return (isGlobal ? "@" : "%") + name;
}

shared_ptr<Variable> Variable::copy(shared_ptr<Variable> var)
{
    if (var->type->isInt())
        return VariablePtr(new Int(var->name, var->isGlobal, var->isConst));
    else if (var->type->isFloat())
        return VariablePtr(new Float(var->name, var->isGlobal, var->isConst));
    else if (var->type->isArr())
        return VariablePtr(new Arr(var->name, var->isGlobal, var->isConst, var->type));
    else
        return VariablePtr(new Ptr(var->name, var->isGlobal, var->isConst, var->type));
}

Int::Int(string name, bool isGlobal, bool isConst, ValuePtr value) : Variable{Type::getInt(), name, isGlobal, isConst}
{
    assert(value->type->isInt() || value->type->isFloat());
    if (value->type->isInt())
        intVal = dynamic_cast<Const *>(value.get())->intVal;
    else
        intVal = dynamic_cast<Const *>(value.get())->floatVal;
}

void Int::print()
{
    assert(isGlobal);
    cout << "@" << name << " = " << (isConst ? "constant" : "global") << " " << type->getStr() << " " << to_string(intVal) << endl;
}

void Int::printHelper()
{
    cout << type->getStr() << " " << to_string(intVal);
}

bool Int::zero()
{
    return !intVal;
}

Float::Float(string name, bool isGlobal, bool isConst, ValuePtr value) : Variable{Type::getFloat(), name, isGlobal, isConst}
{
    assert(value->type->isInt() || value->type->isFloat());
    if (value->type->isInt())
        floatVal = dynamic_cast<Const *>(value.get())->intVal;
    else
        floatVal = dynamic_cast<Const *>(value.get())->floatVal;
}

void Float::print()
{
    assert(isGlobal);
    cout << "@" << name << " = " << (isConst ? "constant" : "global") << " " << type->getStr() << " ";
    union
    {
        double f;
        uint64_t u;
    } f2u;
    f2u.f = floatVal;
    printf("0x%" PRIx64 "\n", f2u.u);
}

void Float::printHelper()
{
    cout << type->getStr() << " ";
    union
    {
        double f;
        uint64_t u;
    } f2u;
    f2u.f = floatVal;
    printf("0x%" PRIx64, f2u.u);
}

bool Float::zero()
{
    return std::abs(floatVal) < 1e-6;
}

void Ptr::print()
{
    assert(0);
}

void Ptr::printHelper()
{
}

bool Ptr::zero()
{
    return false;
}

bool Arr::push(VariablePtr variable)
{
    if (getElementType()->operator==(variable->type))
    {
        if (inner.size() < getElementLength())
        {
            inner.emplace_back(variable);
            return true;
        }
        else
            return false;
    }
    else
    {
        if (inner.size() <= getElementLength())
        {
            if (inner.size() && dynamic_cast<Arr *>(inner.back().get())->push(variable))
            {
                return true;
            }
            else
            {
                if (inner.size() < getElementLength())
                {
                    inner.emplace_back(VariablePtr(new Arr(name, isGlobal, isConst, getElementType())));
                    return dynamic_cast<Arr *>(inner.back().get())->push(variable);
                }
                else
                    return false;
            }
            return false;
        }
        else
            return false;
    }
}

void Arr::fill()
{
    auto sonType = getElementType();
    int i = 0;
    for (; i < inner.size(); i++)
    {
        if (sonType->isArr())
            dynamic_cast<Arr *>(inner[i].get())->fill();
    }
    for (; i < getElementLength(); i++)
    {
        if (sonType->isArr())
        {
            auto tmp = shared_ptr<Arr>(new Arr(name, isGlobal, true, sonType));
            tmp->fill();
            inner.emplace_back(tmp);
        }
        else
        {
            if(sonType->isInt()) inner.emplace_back(VariablePtr(new Int(name, isGlobal, true, 0)));
            else if(sonType->isFloat()) inner.emplace_back(VariablePtr(new Float(name, isGlobal, true, 0)));
        }
    }
}

void Arr::print()
{
    cout << "@" << name << " = " << (isConst ? "constant" : "global") << " ";
    printHelper();
    cout << endl;
}

void Arr::printHelper()
{
    cout << type->getStr();
    if (zero())
        cout << " zeroinitializer";
    else
    {
        cout << " [";
        int i = 0;
        for (; i < inner.size(); i++)
        {
            if (i)
                cout << ", ";
            inner[i]->printHelper();
        }
        for (; i < getElementLength(); i++)
        {
            cout << ", ";
            if (getElementType()->isArr())
            {
                VariablePtr(new Arr(name, isGlobal, true, getElementType()))->printHelper();
            }
            else if(getElementType()->isInt())
            {
                VariablePtr(new Int(name, isGlobal, false, 0))->printHelper();
            }else if(getElementType()->isFloat()){
                VariablePtr(new Float(name, isGlobal, false, 0))->printHelper();
            }
        }
        cout << "]";
    }
}

bool Arr::zero()
{
    if (inner.empty())
        return true;
    for (auto i : inner)
        if (!(i->zero()))
            return false;
    return true;
}
