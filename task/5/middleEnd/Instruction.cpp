#include "Instruction.h"

int CallInstruction::callRegNum = 0;
int BinaryInstruction::BinaryRegNum = 0;
int FnegInstruction::FnegRegNum = 0;
int GetElementPtrInstruction::arrayIdxNum = 0;
int GetElementPtrInstruction::arrayElementNum = 0;
int IcmpInstruction::cmpRegNum = 0;
int PhiInstruction::phiRegNum = 0;
int BrInstruction::ifThenNum = 0;
int BrInstruction::ifElseNum = 0;
int BrInstruction::ifEndNum = 0;
int BrInstruction::orNum = 0;
int BrInstruction::andNum = 0;
int BrInstruction::whileCondNum = 0;
int BrInstruction::whileBodyNum = 0;
int BrInstruction::whileEndNum = 0;
int ExtInstruction::extNum = 0;
int SitofpInstruction::convNum = 0;
// maybe wrong
map<string, IcmpKind> IcmpInstruction::kindMap{{"==", ICmpEQ}, {"!=", ICmpNE}, {"<", ICmpSLT},
                             {"<=", ICmpSLE}, {">", ICmpSGT}, {">=", ICmpSGE}};


void Instruction::moveBefore(shared_ptr<Instruction> I, shared_ptr<Instruction> This){
    if(I == This){
        return;
    }
    auto bb = I->basicblock;
    basicblock->removeInsturction(This);
    bb->insertInstruction(This, I);
}

void Instruction::setName(string newName)
{
    if (reg)
        reg->name = newName;
    else
        assert(false && "cannot setName without reg");
}
string Instruction::getName()
{
    if (reg)
        return reg->name;
    else
        assert(false && "cannot getName without reg");
}

void Instruction::replaceAllUsesWith(ValuePtr V)
{
    assert(false && "replaceAllUseWith donnot finish");
}
void Instruction::replaceAllUsesWith(shared_ptr<Instruction> I)
{
    replaceAllUsesWith(I->reg);
}

// to-do   O(n)得想想办法优化，可以建立一个哈希表映射
vector<shared_ptr<Instruction>>::iterator Instruction::getIterator()
{
    for (auto it = basicblock->instructions.begin(); it != basicblock->instructions.end(); it++)
    {
        if ((*it).get() == this)
        {
            return it;
        }
    }
    assert(false && "getIterator error 你迭代器呢");
    return vector<shared_ptr<Instruction>>().end();
}

void Instruction::deleteSelfInBB()
{
    for (auto it = basicblock->instructions.begin(); it != basicblock->instructions.end(); it++)
    {
        if ((*it).get() == this)
        {
            basicblock->instructions.erase(it);
            break;
        }
    }
}

// I为insertbefore //insert不了，还是在外面insert吧
BinaryInstruction::BinaryInstruction(ValuePtr a, ValuePtr b, char op, Instruction *I) : Instruction{InsID::Binary, I->basicblock, getBinaryReg(a->type)}, a{a}, b{b}, op{op}
{
    // cerr<<"create Binary\n";
    // cerr<<b->name<<endl;
    // cerr<<"b location"<<b.get()<<endl;
    // if(b->I) cerr<<b->I->reg->name<<endl;
    // if(b->I) cerr<<b->I<<endl;
    // cerr<<"this    "<<this<<endl;

    newUse(a.get(), this, reg.get());
    newUse(b.get(), this, reg.get());
    reg->I = this;
    if (op == '!')
        reg->type = Type::getBool();
    // cerr << "[binarypassed] Step1 " << endl;

    if (a->type->isInt() && b->type->isInt() && (op == '+' || op == '*' || op == '!'))
    {
        isCommutative = true;
        isAssociative = true;
    }

    // cerr << "[binarypassed] Step2 " << endl;
    if(a->type->isInt()&&b->type->isInt()&&op == '!'){
        isNilpotent = true;
    }
    // cerr << "[binarypassed] Step3 " << endl;

    // cerr<<"create Binary2\n";
    // cerr<<b->name<<endl;
    // if(b->I) cerr<<b->I->reg->name<<endl;

    // auto &bb = I->basicblock;
    // bb->insertInstruction(this->getSharedThis(),I);
}

void ReturnInstruction::print()
{
    cout << "  ret " << retValue->getTypeStr() << endl;
}

void AllocaInstruction::print()
{
    cout << "  " << des->getStr() << " = alloca " << des->type->getStr() << endl;
}

void StoreInstruction::print()
{
    if (gep)
    {
        cout << "  store " << value->getTypeStr() << ", " << des->type->getStr() << "* getelementptr inbounds (" << gep->from->type->getStr() << ", " << gep->from->getPointStr();
        for (auto ind : gep->index)
            cout << ", " << ind->getTypeStr();
        cout << ")" << endl;
    }
    else
    {
        cout << "  store " << value->getTypeStr() << ", " << des->getPointStr() << endl;
    }
}

void LoadInstruction::print()
{
    if (gep)
    {
        cout << "  " << to->getStr() << " = load " << from->type->getStr() << ", " << from->type->getStr() << "* getelementptr inbounds (" << gep->from->type->getStr() << ", " << gep->from->getPointStr();
        for (auto ind : gep->index)
            cout << ", " << ind->getTypeStr();
        cout << ")" << endl;
    }
    else
    {
        cout << "  " << to->getStr() << " = load " << from->type->getStr() << ", " << from->getPointStr() << endl;
    }
}

void BitCastInstruction::print()
{
    cout << "  " << reg->getStr() << " = bitcast " << from->getPointStr() << " to " << toType->getStr() << endl;
}

void CallInstruction::print()
{
    cout << "  ";
    if (!func->retVal->type->isVoid())
        cout << reg->getStr() << " = ";
    cout << "call " << func->getTypeStr() << "(";
    for (int i = 0; i < argv.size(); i++)
    {
        if (i)
            cout << ", ";
        cout << argv[i]->getTypeStr();
    }
    cout << ")" << endl;
}

void ExtInstruction::print()
{
    cout << "  " << reg->getStr() << " = " << (isign ? "s" : "z") << "ext " << from->getTypeStr() << " to " << to->getStr() << endl;
}

void SitofpInstruction::print()
{
    cout << "  " << reg->getStr() << " = sitofp " << from->getTypeStr() << " to float" << endl;
}

void FptosiInstruction::print()
{
    cout << "  " << reg->getStr() << " = fptosi " << from->getTypeStr() << " to i32" << endl;
}

void PhiInstruction::print()
{

    cout << "  " << reg->getStr() << " = phi " << val->type->getStr() << " ";

    for (int i = 0; i < from.size(); i++)
    {
        cout << "[ " << from[i].first->getStr() << ", %" << from[i].second->label->name << "]";
        if (i != from.size() - 1)
        {
            cout << ", ";
        }
        else
        {
            cout << endl;
        }
    }
}

CallInstruction::CallInstruction(shared_ptr<Function> func, vector<ValuePtr> argv, shared_ptr<BasicBlock> bb) : Instruction{InsID::Call, bb, getCallReg(func->retVal->type)}, func{func}, argv{argv}
{
    for (int i = 0; i < argv.size(); i++)
    {
        newUse(argv[i].get(), this);
    }
    // reg即这条指令本身
    reg->I = this;
}

void GetElementPtrInstruction::print()
{
    if (from->type->isPtr())
    {
        cout << "  " << reg->getStr() << " = getelementptr inbounds " << ((PtrType *)(from->type.get()))->inner->getStr() << ", " << from->getTypeStr();
    }
    else
    {
        cout << "  " << reg->getStr() << " = getelementptr inbounds " << from->type->getStr() << ", " << from->getPointStr();
    }
    for (auto ind : index)
        cout << ", " << ind->getTypeStr();
    cout << endl;
}

void BinaryInstruction::print()
{
    cout << "  " << reg->getStr() << " = ";
    if (a->type->isInt() || a->type->isBool())
    {
        switch (op)
        {
        case '+':
            cout << "add nsw ";
            break;
        case '-':
            cout << "sub nsw ";
            break;
        case '*':
            cout << "mul nsw ";
            break;
        case '/':
            cout << "sdiv ";
            break;
        case '%':
            cout << "srem ";
            break;
        case '!':
            cout << "xor ";
            break;
        case ',':
            cout << "shl ";
            break;
        case '.':
            cout << "ashr ";
            break;
        default:
            cout << "unknow op!";
            break;
        }
    }
    else if (a->type->isFloat())
    {
        switch (op)
        {
        case '+':
            cout << "fadd ";
            break;
        case '-':
            cout << "fsub ";
            break;
        case '*':
            cout << "fmul ";
            break;
        case '/':
            cout << "fdiv ";
            break;
        default:
            cout << "unknow op!";
            break;
        }
    }

    cout << a->type->getStr() << " " << a->getStr() << ", " << b->getStr() << endl;
}

void FnegInstruction::print()
{
    cout << "  " << reg->getStr() << " = fneg " << a->getTypeStr() << endl;
}

void IcmpInstruction::print()
{
    cout << "  " << reg->getStr() << " = icmp ";
    if (op == "!=")
    {
        cout << "ne ";
    }
    else if (op == ">")
    {
        cout << "sgt ";
    }
    else if (op == ">=")
    {
        cout << "sge ";
    }
    else if (op == "<")
    {
        cout << "slt ";
    }
    else if (op == "<=")
    {
        cout << "sle ";
    }
    else if (op == "==")
    {
        cout << "eq ";
    }
    cout << a->getTypeStr() << ", " << b->getStr() << endl;
}

void FcmpInstruction::print()
{
    cout << "  " << reg->getStr() << " = fcmp ";
    if (op == "!=")
    {
        cout << "one ";
    }
    else if (op == ">")
    {
        cout << "ogt ";
    }
    else if (op == ">=")
    {
        cout << "oge ";
    }
    else if (op == "<")
    {
        cout << "olt ";
    }
    else if (op == "<=")
    {
        cout << "ole ";
    }
    else if (op == "==")
    {
        cout << "oeq ";
    }
    cout << a->getTypeStr() << ", " << b->getStr() << endl;
}

void BrInstruction::print()
{
    if (exp)
        cout << "  br " << exp->getTypeStr() << ", " << label_true->getStr() << ", " << label_false->getStr() << endl;
    else
        cout << "  br " << label_true->getStr() << endl;
}

bool ReturnInstruction::replaceValue(ValuePtr target, ValuePtr newValue)
{
    if (retValue == target)
    {
        retValue = newValue;
        return true;
    }
    return false;
}

bool AllocaInstruction::replaceValue(ValuePtr target, ValuePtr newValue)
{
    return false;
}

bool StoreInstruction::replaceValue(ValuePtr target, ValuePtr newValue)
{
    if (gep)
        gep->replaceValue(target, newValue);
    if (value == target)
    {
        value = newValue;
        return true;
    }
    else if (des == target)
    {
        des = newValue;
        return true;
    }
    return false;
}

bool LoadInstruction::replaceValue(ValuePtr target, ValuePtr newValue)
{
    if (gep)
        gep->replaceValue(target, newValue);
    if (from == target)
    {
        from = newValue;
        return true;
    }
    return false;
}

bool BitCastInstruction::replaceValue(ValuePtr target, ValuePtr newValue)
{
    if (from == target)
    {
        from = newValue;
        return true;
    }
    return false;
}

bool CallInstruction::replaceValue(ValuePtr target, ValuePtr newValue)
{
    bool flag = false;
    for (int i = 0; i < argv.size(); i++)
    {
        if (argv[i] == target)
        {
            argv[i] = newValue;
            flag = true;
        }
    }
    return flag;
}

bool ExtInstruction::replaceValue(ValuePtr target, ValuePtr newValue)
{
    if (from == target)
    {
        from = newValue;
        return true;
    }
    return false;
}

bool SitofpInstruction::replaceValue(ValuePtr target, ValuePtr newValue)
{
    if (from == target)
    {
        from = newValue;
        return true;
    }
    return false;
}

bool FptosiInstruction::replaceValue(ValuePtr target, ValuePtr newValue)
{
    if (from == target)
    {
        from = newValue;
        return true;
    }
    return false;
}

bool PhiInstruction::replaceValue(ValuePtr target, ValuePtr newValue)
{
    bool flag = false;
    for (int i = 0; i < from.size(); i++)
    {
        if (from[i].first == target)
        {
            from[i].first = newValue;
            flag = true;
        }
    }
    return flag;
}

bool GetElementPtrInstruction::replaceValue(ValuePtr target, ValuePtr newValue)
{
    bool flag = false;
    if (target == from)
    {
        from = newValue;
        return true;
    }
    for (int i = 0; i < index.size(); i++)
    {
        if (index[i] == target)
        {
            index[i] = newValue;
            flag = true;
        }
    }
    return flag;
}

bool BinaryInstruction::replaceValue(ValuePtr target, ValuePtr newValue)
{
    if (a == target)
    {
        a = newValue;
        return true;
    }
    else if (b == target)
    {
        b = newValue;
        return true;
    }
    std::cerr << "Binary rep ok" << endl;
    return false;
}

bool FnegInstruction::replaceValue(ValuePtr target, ValuePtr newValue)
{
    if (a == target)
    {
        a = newValue;
        return true;
    }

    return false;
}

bool IcmpInstruction::replaceValue(ValuePtr target, ValuePtr newValue)
{
    if (a == target)
    {
        a = newValue;
        return true;
    }
    else if (b == target)
    {
        b = newValue;
        return true;
    }
    return false;
}

bool FcmpInstruction::replaceValue(ValuePtr target, ValuePtr newValue)
{
    if (a == target)
    {
        a = newValue;
        return true;
    }
    else if (b == target)
    {
        b = newValue;
        return true;
    }
    return false;
}

bool BrInstruction::replaceValue(ValuePtr target, ValuePtr newValue)
{
    if (exp == target)
    {
        exp = newValue;
        return true;
    }

    return false;
}

void deleteUser(ValuePtr user)
{
    auto I = user->I;
    if(!I){
        return ;
    }
    int numOperands = I->getNumOperands();
    for(int i = 0; i < numOperands; i ++) {
        auto operand = I->getOperand(i);
        Use *use = operand->useHead;
        while(use != nullptr) {
            if(use->user == I) {
                use->rmUse();
            }
            use = use->next;
        }
    }
}


void deleteUser(InstructionPtr user) {
    auto I = user.get();
    if(!I){
        return ;
    }
    int numOperands = I->getNumOperands();
    for(int i = 0; i < numOperands; i ++) {
        auto operand = I->getOperand(i);
        Use *use = operand->useHead;
        while(use != nullptr) {
            if(use->user == I) {
                use->rmUse();
            }
            use = use->next;
        }
    }
}


void deleteUser(Instruction* user) {
    auto I = user;
    if(!I){
        return ;
    }
    int numOperands = I->getNumOperands();
    for (int i = 0; i < numOperands; i++)
    {
        auto operand = I->getOperand(i);
        Use *use = operand->useHead;
        while (use != nullptr)
        {
            if (use->user == I)
            {
                use->rmUse();
            }
            use = use->next;
        }
    }
}


void replaceVarByVar(ValuePtr from, ValuePtr to)
{
    // from已经不会再使用，所以没必要维护from的Use
    // cerr<<"enter replaceVarByVar\n";
    Use *p = from->useHead;
    // bool is_needed = false;
    // if(from->name == "func.binary456.copy0") {
    //     is_needed = true;
    //     printf("replace %s <- %s\n", from->name.c_str(), to->name.c_str());
    // }
    
    while (p != nullptr)
    {   
        // cerr<<"replaceVarByVar1\n";
        auto user = p->user;
        // cerr<<"replaceVarByVar2\n";
        int numOperand = user->getNumOperands();
        // cerr<<"replaceVarByVar3\n";
        // if(is_needed) {
        //     printf("user: ");
        //     user->print();
        // }
        if (!user->replaceValue(from, to))
        {
            std::cerr << user->reg->name << " error " << user->type << "\n";
            // assert(false && "error when replaceVarByVar");
        }
        auto tmp = p->next;
        p->rmUse();
        newUse(to.get(), user);
        p = tmp;
    }
    deleteUser(from);
    // cerr<<"after replaceVarByVar\n";
}

void replaceVarByVarForLCSSA(ValuePtr from, ValuePtr to, Use *use)
{
    // from已经不会再使用，所以没必要维护from的Use
    int cnt = 0;
    Instruction *la = nullptr;
    unordered_map<Instruction *, bool> mp;

    auto user = use->user;
    int numOperand = user->getNumOperands();
    if (!user->replaceValue(from, to))
    {
        cerr << user->reg->name << " error " << user->type << "\n";
        // assert(false && "error when replaceVarByVar");
    }
    use->rmUse();
    newUse(to.get(), user);
    deleteUser(from);
}

unsigned ReturnInstruction::getNumOperands()
{
    return 1;
}
unsigned AllocaInstruction::getNumOperands()
{
    return 1;
}
unsigned GetElementPtrInstruction::getNumOperands()
{
    return 1 + index.size();
}
unsigned StoreInstruction::getNumOperands()
{
    return 2;
}
unsigned LoadInstruction::getNumOperands()
{
    return 1;
}
unsigned BitCastInstruction::getNumOperands()
{
    return 1;
}
unsigned ExtInstruction::getNumOperands()
{
    return 1;
}
unsigned SitofpInstruction::getNumOperands()
{
    return 1;
}
unsigned FptosiInstruction::getNumOperands()
{
    return 1;
}
// 理论上讲，func应该也是，但是因为func的类型不是value，所以就不算了
unsigned CallInstruction::getNumOperands()
{
    return argv.size();
}
unsigned BinaryInstruction::getNumOperands()
{
    return 2;
}
unsigned FnegInstruction::getNumOperands()
{
    return 1;
}

unsigned IcmpInstruction::getNumOperands()
{
    return 2;
}
unsigned FcmpInstruction::getNumOperands()
{
    return 2;
}
// label 算吗
unsigned BrInstruction::getNumOperands()
{
    if (exp != nullptr)
        return 1;
    else
        return 0;
}
// val不应该算
unsigned PhiInstruction::getNumOperands()
{
    return from.size();
}

ValuePtr ReturnInstruction::getOperand(unsigned i)
{
    assert(i == 0 && "returnInstruction getOperand Error\n");
    return retValue;
}
ValuePtr AllocaInstruction::getOperand(unsigned i)
{
    assert(i == 0 && "allocaInstruction getOperand Error\n");
    return des;
}
ValuePtr GetElementPtrInstruction::getOperand(unsigned i)
{
    assert(i <= index.size() && "GetElementPtrInstruction getOperand Error\n");
    if (i == 0)
    {
        return from;
    }
    else
    {
        return index[i - 1];
    }
}
ValuePtr StoreInstruction::getOperand(unsigned i)
{
    assert(i < 2 && "StoreInstruction getOperand Error\n");
    if (i == 0)
    {
        return des;
    }
    else
    {
        return value;
    }
}
ValuePtr LoadInstruction::getOperand(unsigned i)
{
    assert(i == 0 && "LoadInstruction getOperand Error\n");
    return from;
}
ValuePtr BitCastInstruction::getOperand(unsigned i)
{
    assert(i == 0 && "BitCastInstruction getOperand Error\n");
    return from;
}
ValuePtr ExtInstruction::getOperand(unsigned i)
{
    assert(i == 0 && "ExtInstruction getOperand Error\n");
    return from;
}
ValuePtr SitofpInstruction::getOperand(unsigned i)
{
    assert(i == 0 && "SitofpInstruction getOperand Error\n");
    return from;
}

ValuePtr FptosiInstruction::getOperand(unsigned i)
{
    assert(i == 0 && "FptosiInstruction getOperand Error\n");
    return from;
}
ValuePtr CallInstruction::getOperand(unsigned i)
{
    assert(i < argv.size() && "CallInstruction getOperand Error\n");
    return argv[i];
}
ValuePtr BinaryInstruction::getOperand(unsigned i)
{
    assert(i < 2 && "BinaryInstruction getOperand Error\n");
    if (i == 0)
    {
        return a;
    }
    else
    {
        return b;
    }
}
ValuePtr FnegInstruction::getOperand(unsigned i)
{
    assert(i == 0 && "FnegInstruction getOperand Error\n");
    return a;
}

ValuePtr IcmpInstruction::getOperand(unsigned i)
{
    assert(i < 2 && "IcmpInstruction getOperand Error\n");
    if (i == 0)
    {
        return a;
    }
    else
    {
        return b;
    }
}
ValuePtr FcmpInstruction::getOperand(unsigned i)
{
    assert(i < 2 && "FcmpInstruction getOperand Error\n");
    if (i == 0)
    {
        return a;
    }
    else
    {
        return b;
    }
}
ValuePtr BrInstruction::getOperand(unsigned i)
{
    assert(i == 0 && "BrInstruction getOperand Error\n");
    return exp;
}
ValuePtr PhiInstruction::getOperand(unsigned i)
{
    assert(i < from.size() && "PhiInstruction getOperand Error\n");

    return from[i].first;
}

void PhiInstruction::removeIncomingByBB(shared_ptr<BasicBlock> bb)
{
    int indexToRemove = -1;
    for(int i = 0; i < from.size(); i++)
    {
        auto predUse = from.at(i);
        if(predUse.second == bb)
            indexToRemove = i;
    }

    if(indexToRemove >= 0)
    {
        auto use1 = from.at(indexToRemove).first;
        deleteUser(use1);
        from.erase(from.begin()+indexToRemove);

        // for(auto i = 0; i < operands.size(); i++)
        //     operands.at(i)->reSetPos(i);
    }

}

ReturnInstruction::~ReturnInstruction(){
    deleteUser(this);
}
AllocaInstruction::~AllocaInstruction(){
    deleteUser(this);
}
GetElementPtrInstruction::~GetElementPtrInstruction(){
    deleteUser(this);
}
StoreInstruction::~StoreInstruction(){
    deleteUser(this);
}
LoadInstruction::~LoadInstruction(){
    deleteUser(this);
}
BitCastInstruction::~BitCastInstruction(){
    deleteUser(this);
}
ExtInstruction::~ExtInstruction(){
    deleteUser(this);
}
SitofpInstruction::~SitofpInstruction(){
    deleteUser(this);
}
FptosiInstruction::~FptosiInstruction(){
    deleteUser(this);
}

CallInstruction::~CallInstruction(){
    deleteUser(this);
}
BinaryInstruction::~BinaryInstruction(){
    deleteUser(this);
}
FnegInstruction::~FnegInstruction(){
    deleteUser(this);
}

IcmpInstruction::~IcmpInstruction(){
    deleteUser(this);
}
FcmpInstruction::~FcmpInstruction(){
    deleteUser(this);
}

BrInstruction::~BrInstruction(){
    deleteUser(this);
}
PhiInstruction::~PhiInstruction(){
    deleteUser(this);
}