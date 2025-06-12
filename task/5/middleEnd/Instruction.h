#pragma once
#include <cassert>
#include <memory>
#include "Variable.h"
#include "Function.h"
#include "Label.h"
#include "utils.h"
#include <stdlib.h>
#include <map>
#include <iostream>

struct Function;
struct BasicBlock;
struct Instruction;
using std::enable_shared_from_this;


// 所有指令类型
enum InsID
{
    Return,
    Br,
    Alloca,
    Store,
    Load,
    Call,
    Bitcast,
    Ext,
    Sitofp,
    Fptosi,
    GEP,
    Binary,
    Fneg,
    Icmp,
    Fcmp,
    Phi
};


struct Instruction: public enable_shared_from_this<Instruction> {
    // 指令类型
    InsID type;
    // 所属的basicblock
    shared_ptr<BasicBlock> basicblock;
    // 指令的返回值
    ValuePtr reg;

    Instruction(InsID type, shared_ptr<BasicBlock> bb, ValuePtr reg = nullptr): type{type}, basicblock{bb}, reg{reg} {}
    virtual ~Instruction(){}
    virtual void print() {}

    virtual bool replaceValue(ValuePtr target, ValuePtr newValue) { return false; }
    // 返回指令的操作数
    virtual unsigned getNumOperands() { return 0; }
    // 返回第i个操作数
    virtual ValuePtr getOperand(unsigned i) { return nullptr; }
    

    // 用于判断是否满足交换律、结合律、幂等性、幂零性
    bool isCommutative = false;
    bool isAssociative = false;
    bool isIdempotent = false;
    bool isNilpotent = false;

    // 将This指令移动到I指令之前
    void moveBefore(shared_ptr<Instruction> I, shared_ptr<Instruction> This);

    // 将当前指令的返回值替换为V或I的返回值
    void replaceAllUsesWith(ValuePtr V);
    void replaceAllUsesWith(shared_ptr<Instruction> I);
    
    //获得在basicblock的instructions列表中的迭代器
    vector<shared_ptr<Instruction>>::iterator getIterator();

    // 修改指令返回值的名字
    void setName(string newName);
    // 返回指令名字
    string getName();
    // 从basicBlock中删除指令
    void deleteSelfInBB();
    // 返回这个指令的shared_ptr
    shared_ptr<Instruction> getSharedThis() { return shared_from_this(); }
};
typedef shared_ptr<Instruction> InstructionPtr;

// return指令
struct ReturnInstruction:public Instruction {
    // 返回指令的返回值
    ValuePtr retValue;
    ReturnInstruction(ValuePtr retValue, shared_ptr<BasicBlock> bb) : Instruction{InsID::Return, bb}, retValue{retValue} {
        newUse(retValue.get(), this);
    }; 
    ~ReturnInstruction();
    
    // 所有的print函数都是用于输出llvmir
    virtual void print() override;
    // 将返回值替换为newValue
    virtual bool replaceValue(ValuePtr target, ValuePtr newValue) override;

    virtual unsigned getNumOperands() override;
    virtual ValuePtr getOperand(unsigned i) override;
};

struct AllocaInstruction:public Instruction {
    // 分配的内存空间
    ValuePtr des;
    AllocaInstruction(ValuePtr des, shared_ptr<BasicBlock> bb) : Instruction{InsID::Alloca, bb}, des{des} {
        des->I = this;
    };
    ~AllocaInstruction();

    virtual void print() override;
    virtual bool replaceValue(ValuePtr target, ValuePtr newValue) override;

    virtual unsigned getNumOperands() override;
    virtual ValuePtr getOperand(unsigned i) override;
};
// gep指令
struct GetElementPtrInstruction:public Instruction {
    // 
    static int arrayIdxNum;
    static int arrayElementNum;
    static ValuePtr getArrayIdxReg(TypePtr type) { return ValuePtr(new Reg(type, "arrayidx" + to_string(arrayIdxNum++))); }
    static ValuePtr getArrayElementReg(TypePtr type) { return ValuePtr(new Reg(type, "arrayinit.element" + to_string(arrayElementNum++))); }
    ValuePtr from;
    vector<ValuePtr> index;
    ~GetElementPtrInstruction();
    GetElementPtrInstruction(ValuePtr from, vector<ValuePtr> index, shared_ptr<BasicBlock> bb) : Instruction{InsID::GEP, bb}, from{from}, index{index}
    {
        if (index.size() == 1)
        {
            reg = getArrayElementReg(from->type);
            newUse(from.get(), this, reg.get());
            newUse(index[0].get(), this, reg.get()); // bug
        }
        else
        {
            // std::cerr<<index.size()<<"\n";
            auto curr = from->type;
            for (int i = 1; i < index.size(); i++)
            {
                assert(curr->isArr());
                // std::cerr<<dynamic_cast<ArrType *>(curr.get())->getStr()<<"\n";
                curr = dynamic_cast<ArrType *>(curr.get())->inner;
                
                newUse(index[i].get(), this, reg.get());
            }
            reg = getArrayIdxReg(curr);
            newUse(from.get(), this, reg.get());
            newUse(index[0].get(), this, reg.get()); // bug
        }
        reg->I = this;
    }
    virtual void print() override;
    virtual bool replaceValue(ValuePtr target, ValuePtr newValue) override;
    virtual unsigned getNumOperands() override;
    virtual ValuePtr getOperand(unsigned i) override;
};

// store指令
struct StoreInstruction:public Instruction {
    // 从gep或des获取存储的地址（因为没有完全统一value和ins，所以需要留个位）
    ValuePtr des;
    shared_ptr<GetElementPtrInstruction> gep;
    // 存储的值
    ValuePtr value;
    ~StoreInstruction();
    StoreInstruction(shared_ptr<GetElementPtrInstruction> gep, ValuePtr value, shared_ptr<BasicBlock> bb) : Instruction{InsID::Store, bb}, gep{gep}, des{gep->reg}, value{value} {
        newUse(gep->reg.get(), this);
        newUse(value.get(), this);
    };
    
    StoreInstruction(ValuePtr des, ValuePtr value, shared_ptr<BasicBlock> bb) : Instruction{InsID::Store, bb}, des{des}, value{value} {
        newUse(des.get(), this);
        newUse(value.get(), this);
    };
    virtual void print() override;
    virtual bool replaceValue(ValuePtr target, ValuePtr newValue) override;
    virtual unsigned getNumOperands() override;
    virtual ValuePtr getOperand(unsigned i) override;
};

// load指令
struct LoadInstruction:public Instruction {
    // 从gep或from获取加载的地址
    ValuePtr from;
    shared_ptr<GetElementPtrInstruction> gep;
    // 分配的地址
    ValuePtr to;
    LoadInstruction(ValuePtr from, ValuePtr to, shared_ptr<BasicBlock> bb) : Instruction{InsID::Load, bb}, from{from}, to{to} {
        newUse(from.get(), this, to.get());
        //to即这条指令本身
        to->I = this;
        reg = to;
    };
    ~LoadInstruction();
    LoadInstruction(shared_ptr<GetElementPtrInstruction> gep, ValuePtr to, shared_ptr<BasicBlock> bb) : Instruction{InsID::Load, bb}, gep{gep}, from{gep->reg}, to{to} {
        newUse(gep->reg.get(), this, to.get());
        to->I = this;
    };
    virtual void print() override;
    virtual bool replaceValue(ValuePtr target, ValuePtr newValue) override;
    virtual unsigned getNumOperands() override;
    virtual ValuePtr getOperand(unsigned i) override;
};

// 用于类型转换
struct BitCastInstruction:public Instruction {
    // 待转换的源操作数
    ValuePtr from;
    // 目标转换类型
    TypePtr toType;
    ~BitCastInstruction();
    BitCastInstruction(ValuePtr from, ValuePtr reg, shared_ptr<BasicBlock> bb, TypePtr toType = TypePtr(new PtrType(Type::getInt8()))) : Instruction{InsID::Bitcast, bb, reg}, from{from}, toType{toType} {
        newUse(from.get(), this, reg.get());
        //reg即这条指令本身
        reg->I = this;
    }
    virtual void print() override;
    virtual bool replaceValue(ValuePtr target, ValuePtr newValue) override;
    virtual unsigned getNumOperands() override;
    virtual ValuePtr getOperand(unsigned i) override;
};

// 对应zext(零扩展)和sext(符号扩展)指令
struct ExtInstruction:public Instruction {
    // 保证命名不重复
    static int extNum;
    static ValuePtr getExtReg(TypePtr type) { return ValuePtr(new Reg(type, "ext" + to_string(extNum++))); }
    ~ExtInstruction();
    // 待扩展的源操作数
    ValuePtr from;
    // 目标类型
    TypePtr to;
    // 是否为符号扩展
    bool isign;
    ExtInstruction(ValuePtr from, TypePtr to, bool isign, shared_ptr<BasicBlock> bb) : Instruction{InsID::Ext, bb, getExtReg(to)}, from{from}, to{to}, isign{isign} {
        newUse(from.get(), this, reg.get());
        //reg即这条指令本身
        reg->I = this;
    }
    virtual void print() override;
    virtual bool replaceValue(ValuePtr target, ValuePtr newValue) override;
    virtual unsigned getNumOperands() override;
    virtual ValuePtr getOperand(unsigned i) override;
};

// 用于将有符号整数转换为浮点数
struct SitofpInstruction:public Instruction {
    // 保证命名不重复
    static int convNum;
    static ValuePtr getConvReg(TypePtr type) { return ValuePtr(new Reg(type, "conv" + to_string(convNum++))); }
    // 待转换的源操作数
    ValuePtr from;
    ~SitofpInstruction();
    SitofpInstruction(ValuePtr from, shared_ptr<BasicBlock> bb) : Instruction{InsID::Sitofp, bb, getConvReg(Type::getFloat())}, from{from} {
        newUse(from.get(), this, reg.get());
        //reg即这条指令本身
        reg->I = this;
    }
    virtual void print() override;
    virtual bool replaceValue(ValuePtr target, ValuePtr newValue) override;
    virtual unsigned getNumOperands() override;
    virtual ValuePtr getOperand(unsigned i) override;

    friend struct FptosiInstruction;
};

// 用于将浮点数转换为有符号整数
struct FptosiInstruction:public Instruction {
    // 源操作数
    ValuePtr from;
    ~FptosiInstruction();
    FptosiInstruction(ValuePtr from, shared_ptr<BasicBlock> bb) : Instruction{InsID::Fptosi, bb, SitofpInstruction::getConvReg(Type::getInt())}, from{from} {
        newUse(from.get(), this, reg.get());
        //reg即这条指令本身
        reg->I = this;
    }
    virtual void print() override;
    virtual bool replaceValue(ValuePtr target, ValuePtr newValue) override;
    virtual unsigned getNumOperands() override;
    virtual ValuePtr getOperand(unsigned i) override;
};

struct CallInstruction:public Instruction {
    static int callRegNum;
    static ValuePtr getCallReg(TypePtr type) { return ValuePtr(new Reg(type, "call" + to_string(callRegNum++))); }
    shared_ptr<Function> func;
    vector<ValuePtr> argv;
    ~CallInstruction();
    CallInstruction(shared_ptr<Function> func, vector<ValuePtr> argv, shared_ptr<BasicBlock> bb);
    virtual void print() override;
    virtual bool replaceValue(ValuePtr target, ValuePtr newValue) override;
    virtual unsigned getNumOperands() override;
    virtual ValuePtr getOperand(unsigned i) override;
};

// 二元运算指令类型
enum BinaryInstructionOps
{
    Add,
    Mul,
    Sub,
    Div,
    Rem,
    Xor,
    Shl,
    AShr,
    FAdd,
    FSub,
    FMul,
    FDiv
};

// 二元运算指令
struct BinaryInstruction : public Instruction
{
    // 用于命名，防止重复
    static int BinaryRegNum;
    static ValuePtr getBinaryReg(TypePtr type) { return ValuePtr(new Reg(type, "binary" + to_string(BinaryRegNum++))); }
    // 两个操作数
    ValuePtr a;
    ValuePtr b;
    // 运算类型
    char op;
    ~BinaryInstruction();
    BinaryInstruction(ValuePtr a, ValuePtr b, char op, shared_ptr<BasicBlock> bb) : Instruction{InsID::Binary, bb, getBinaryReg(a->type)}, a{a}, b{b}, op{op}
    {
        // std::cerr << "new Binary is here" << std::endl;
        newUse(a.get(), this, reg.get());
        // std::cerr << "new Binary step0" << std::endl;
        newUse(b.get(), this, reg.get());
        // std::cerr << "new Binary step1" << std::endl;
        reg->I = this;
        if (op == '!')
            reg->type = Type::getBool();

        if(a->type->isInt()&&b->type->isInt()&&(op == '+'||op == '*'||op == '!')){
            isCommutative = true;
            isAssociative = true;
        }
        if(a->type->isInt()&&b->type->isInt()&&op == '!'){
            isNilpotent = true;
        }
        // std::cerr << "new Binary step2" << std::endl;
    };

    //I为insertbefore
    BinaryInstruction(ValuePtr a, ValuePtr b, char op, Instruction* I);

    // 交换操作数
    void swapOperands(){
        ValuePtr temp;
        temp = a;
        a = b; 
        b = temp;
    }
    // 设置操作数
    void setOperands(unsigned index, ValuePtr newOp){
        assert(index<2&&"binaryInstruction only has two operands");
        assert(!reg->isConst&&!(dynamic_cast<Variable*>(newOp.get())&&dynamic_cast<Variable*>(newOp.get())->isGlobal)&&"cannot mutate a constant with setOperand");
        if(index == 0){
            Use *use = a->useHead;
            while(use) {
                if(use->user == this) {
                    use->rmUse();
                    break;
                }
                use = use->next;
            }
            a = newOp;
            newUse(newOp.get(),this,reg.get());
        }
        else if(index == 1){
            Use *use = b->useHead;
            while(use) {
                if(use->user == this) {
                    use->rmUse();
                    break;
                }
                use = use->next;
            }
            b = newOp;
            newUse(newOp.get(),this,reg.get());
        }
    }

    // 获取操作数
    unsigned getOpcode(){
        if (a->type->isInt() || a->type->isBool())
        {
            switch (op)
            {
            case '+':
                return Add;
                break;
            case '-':
                return Sub;
                break;
            case '*':
                return Mul;
                break;
            case '/':
                return Div;
                break;
            case '%':
                return Rem;
                break;
            case '!':
                return Xor;
                break;
            case ',':
                return Shl;
                break;
            case '.':
                return AShr;
                break;

            default:
                assert(false&&"Binary type errpr");
                break;
            }
        }
        else if (a->type->isFloat())
        {
            switch (op)
            {
            case '+':
                return FAdd;
                break;
            case '-':
                return FSub;
                break;
            case '*':
                return FMul;
                break;
            case '/':
                return FDiv;
                break;
            default:
                assert(false&&"Binary Float type error");
                break;
            }
        }
        return 0;
    }

    

    virtual void print() override;
    virtual bool replaceValue(ValuePtr target, ValuePtr newValue) override;
    virtual unsigned getNumOperands() override;
    virtual ValuePtr getOperand(unsigned i) override;
};

// fneg指令
struct FnegInstruction:public Instruction {
    // 保证不重复
    static int FnegRegNum;
    static ValuePtr getFnegReg() { return ValuePtr(new Reg(Type::getFloat(), "fneg" + to_string(FnegRegNum++))); }
    // 待取负的操作数
    ValuePtr a;
    ~FnegInstruction();
    FnegInstruction(ValuePtr a, shared_ptr<BasicBlock> bb) : Instruction{InsID::Fneg, bb, getFnegReg()}, a{a} {
        newUse(a.get(), this, reg.get());
        reg->I = this;
    }
    virtual void print() override;
    virtual bool replaceValue(ValuePtr target, ValuePtr newValue) override;
    virtual unsigned getNumOperands() override;
    virtual ValuePtr getOperand(unsigned i) override;
};

// icmp指令，整数的比较
struct IcmpInstruction:public Instruction {
    static int cmpRegNum; // 这里本来应该是 block 来处理，我看 llvm ir 不会报错就偷懒没改了
    static std::map<string, IcmpKind> kindMap;
    static ValuePtr getCmpReg() { return ValuePtr(new Reg(Type::getBool(), "cmp" + to_string(cmpRegNum++))); }
    // 比较指令的操作数
    ValuePtr a;
    ValuePtr b;
    // 比较操作符的字符形态
    string op;
    // 比较操作符的枚举形态，与上者是同样的东西
    IcmpKind kind;

    ~IcmpInstruction();
    IcmpInstruction(shared_ptr<BasicBlock> bb, ValuePtr a, ValuePtr b = Const::getConst(Type::getInt(), 0)/*ValuePtr(new Const(Type::getInt(), 0))*/, string op = "!=") : Instruction{InsID::Icmp, bb, getCmpReg()}, a{a}, b{b}, op{op} {
        newUse(a.get(), this, reg.get());
        newUse(b.get(), this, reg.get());
        reg->I = this;
        kind = kindMap[op];
    };
    
    virtual void print() override;
    virtual bool replaceValue(ValuePtr target, ValuePtr newValue) override;
    virtual unsigned getNumOperands() override;
    virtual ValuePtr getOperand(unsigned i) override;

    friend struct FcmpInstruction;
};

// 浮点数比较指令
struct FcmpInstruction:public Instruction {
    // 操作数
    ValuePtr a;
    ValuePtr b;
    // 操作符
    string op;
    ~FcmpInstruction();
    FcmpInstruction(shared_ptr<BasicBlock> bb, ValuePtr a, ValuePtr b = Const::getConst(Type::getFloat(), (float)0)/*ValuePtr(new Const(Type::getFloat(), (float)0))*/, string op = "!=") : Instruction{InsID::Fcmp, bb, IcmpInstruction::getCmpReg()}, a{a}, b{b}, op{op} {
        newUse(a.get(), this, reg.get());
        newUse(b.get(), this, reg.get());
        reg->I = this;
    };
    virtual void print() override;
    virtual bool replaceValue(ValuePtr target, ValuePtr newValue) override;
    virtual unsigned getNumOperands() override;
    virtual ValuePtr getOperand(unsigned i) override;
};

// 跳转指令
struct BrInstruction:public Instruction {
    // 用于命名不重复
    static int ifThenNum;
    static int ifEndNum;
    static int ifElseNum;
    static int orNum;
    static int andNum;
    static int whileCondNum;
    static int whileBodyNum;
    static int whileEndNum;
    static string getifThenStr() { return "if.then" + to_string(ifThenNum++); }
    static string getifEndStr() { return "if.end" + to_string(ifEndNum++); }
    static string getifElseStr() { return "if.else" + to_string(ifElseNum++); }
    static string getOrStr() { return "lor.lhs.false" + to_string(orNum++); }
    static string getAndStr() { return "land.lhs.true" + to_string(andNum++); }
    static string getWhileCondStr() { return "while.cond" + to_string(whileCondNum++); }
    static string getwhileBodyStr() { return "while.body" + to_string(whileBodyNum++); }
    static string getwhileEndStr() { return "while.end" + to_string(whileEndNum++); }
    // 跳转条件
    ValuePtr exp = nullptr;
    // 跳转目标块的label
    LabelPtr label_true;
    LabelPtr label_false;
    ~BrInstruction();
    BrInstruction(ValuePtr exp, LabelPtr label_true, LabelPtr label_false, shared_ptr<BasicBlock> bb) : Instruction{InsID::Br, bb}, exp{exp}, label_true{label_true}, label_false{label_false} {
        newUse(exp.get(), this);
    }
    BrInstruction(LabelPtr label, shared_ptr<BasicBlock> bb) : Instruction{InsID::Br, bb}, label_true{label} {}
    virtual void print() override;
    virtual bool replaceValue(ValuePtr target, ValuePtr newValue) override;
    virtual unsigned getNumOperands() override;
    virtual ValuePtr getOperand(unsigned i) override;
};
// phi指令
struct PhiInstruction:public Instruction {
    // 用于命名不重复
    static int phiRegNum; 
    static ValuePtr getPhiReg(TypePtr type) { return ValuePtr(new Reg(type, "phi" + to_string(phiRegNum++))); }
    ValuePtr val; // mem2reg中才会用到
    // 跳转的块以及对应的值
    // 例如：对于 %x = phi i32 [ 1, %bb1 ], [ 2, %bb2 ]
    vector<std::pair<ValuePtr,shared_ptr<BasicBlock>>> from;
    string op;
    ~PhiInstruction();
    PhiInstruction(shared_ptr<BasicBlock> bb, ValuePtr val) : Instruction{InsID::Phi, bb, getPhiReg(val->type)}, val{val} {
        reg->I = this;
    }; 
    // addFrom单独加就行
    /*
    a   d
    |   |
    b   e
    |   |
    c---|
    |
    f
    pred 必须是c的前置基本块，这里是b和e
    */
   // 对应关系里添加块值对
    void addFrom(ValuePtr value, shared_ptr<BasicBlock> pred){
        from.push_back({value,pred});
        newUse(value.get(), this, reg.get());
    }
    // 删除一个前置基本块
    void removeIncomingByBB(shared_ptr<BasicBlock> bb);
    virtual void print() override;
    virtual bool replaceValue(ValuePtr target, ValuePtr newValue) override;
    virtual unsigned getNumOperands() override;
    virtual ValuePtr getOperand(unsigned i) override;
};


void replaceVarByVar(ValuePtr from, ValuePtr to);
void replaceVarByVarForLCSSA(ValuePtr from, ValuePtr to, Use* use); 
void deleteUser(ValuePtr user);
void deleteUser(InstructionPtr user);
void deleteUser(Instruction* user);