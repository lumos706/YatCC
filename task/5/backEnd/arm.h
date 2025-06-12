#ifndef ARM_H
#define ARM_H

#include <string.h>

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h> // vprintf printf
#include <stdlib.h> // exit
#include <assert.h>
#include <utility> // std::pair
#include <map>
#include <set>
#include <queue>
#include<iostream>

#include <cstring> // strcpy, strlen

#include "array.h"
#include "general.h"
#include "Module.h"
#include "Loop.h"

using namespace std;



enum Int_Reg : uint8 {
    r0 = 0,
    r1 = 1,
    r2 = 2,
    r3 = 3,
    r4 = 4,
    r5 = 5,
    r6 = 6,
    r7 = 7,
    r8 = 8,
    r9 = 9,
    r10 = 10,
    r11 = 11,

    r12 = 12,
    r13 = 13,
    r14 = 14,
    r15 = 15,
    REG_COUNT = 16,

    // alias
    fp = r11,
    ip = r12,
    sp = r13,
    lr = r14,
    pc = r15,

};
// 浮点数寄存器
enum float_reg : uint8{
    s0 = 0,
    s1 = 1,
    s2 = 2,
    s3 = 3,
    s4 = 4,
    s5 = 5,
    s6 = 6,
    s7 = 7,
    s8 = 8,
    s9 = 9,
    s10 = 10,
    s11 = 11,
    s12 = 12,
    s13 = 13,
    s14 = 14,
    s15 = 15,
    s16 = 16,
    s17 = 17,
    s18 = 18,
    s19 = 19,
    s20 = 20,
    s21 = 21,
    s22 = 22,
    s23 = 23,
    s24 = 24,
    s25 = 25,
    s26 = 26,
    s27 = 27,
    s28 = 28,
    s29 = 29,
    s30 = 30,
    s31 = 31,
    SREG_COUNT = 32
};

/**
 * ARM指令的二进制操作类型
 * 这些表示可以对两个操作数执行的不同操作
 */
enum Binary_Op_Type {
	BINARY_ADD,               // 加法操作
	BINARY_SUBTRACT,          // 减法操作
	BINARY_MULTIPLY,          // 乘法操作
	BINARY_SMMUL,             // 有符号最高有效字乘法
	BINARY_DIVIDE,            // 除法操作
	BINARY_MOD,               // 取模操作
    F_NEG,                    // 浮点数取反

	/* ASM 二进制指令 */
	BINARY_LSL,               // 逻辑左移
	BINARY_LSR,               // 逻辑右移
	BINARY_ASL,               // 算术左移
	BINARY_ASR,               // 算术右移
	BINARY_RSB,               // 反向减法（用于机器指令）
	BINARY_BITWISE_AND,       // 位与操作
	BINARY_BITWISE_OR,        // 位或操作
	BINARY_BIC,               // 位清除操作（AND NOT）

    BINARY_LOGICAL_AND,       // 逻辑与（布尔）
    BINARY_LOGICAL_OR,        // 逻辑或（布尔）
    BINARY_NOT_EQUAL_TO,      // 比较：不等于
    BINARY_EQUAL_TO,          // 比较：等于
    BINARY_LESS_THAN_OR_EQUAL_TO,    // 比较：小于等于
    BINARY_GREATER_THAN_OR_EQUAL_TO, // 比较：大于等于
    BINARY_LESS_THAN,         // 比较：小于
    BINARY_GREATER_THAN,      // 比较：大于
    UNARY_XOR,                // 位异或（一元操作）
    FLOAT_GE                  // 浮点数大于等于比较
};

/**
 * 复杂乘法操作类型
 * 这些表示涉及带有额外操作数的乘法运算
 */
enum ComplexMul_Op_Type {
    COMPLEX_MLA,              // 乘加：Rd = Ra + (Rn * Rm)
    COMPLEX_MLS               // 乘减：Rd = Ra - (Rn * Rm)
};

/**
 * 操作数标签类型
 * 这些标识指令中使用的操作数类型
 */
enum Operand_Tag {
	//UNDEFINED,
	ERRORTYPE,                // 错误或未定义的操作数类型
	REG,                      // 物理寄存器，已分配或预分配
	VREG,                     // 虚拟寄存器，等待分配
    SREG,                     // 物理浮点寄存器
    VSREG,                    // 虚拟浮点寄存器
	IMM,                      // 立即整数值
    SIMM,                     // 立即浮点值
	ADR_GLOBAL                // 全局地址引用
};

/**
 * 移位操作类型
 * 这些表示ARM指令中可用的不同移位操作
 */
enum Shift_Tag {
	Nothing,                  // 无移位操作
	LSL,                      // 逻辑左移
	LSR,                      // 逻辑右移
	ASR,                      // 算术右移
	ASL                       // 算术左移
};

/**
 * 分支条件代码
 * 这些表示可能触发分支的不同条件
 */
enum Branch_Condition {
    NO_CONDITION          = 0,  // 无条件分支

    LESS_THAN             = 1,  // 如果小于则分支
    GREATER_THAN          = 2,  // 如果大于则分支
    NOT_EQUAL             = 3,  // 如果不等则分支

    GREATER_THAN_OR_EQUAL = 4,  // 如果大于等于则分支
    LESS_THAN_OR_EQUAL    = 5,  // 如果小于等于则分支
    EQUAL                 = 6,  // 如果等于则分支
    float_ge              = 7,  // 如果浮点数大于等于（正数或零）则分支
    float_lt              = 8,  // 如果浮点数小于则分支
    float_ne              = 9   // 如果浮点数不等则分支
};

struct MI_Use;
struct MI;
struct Machine_Block;
struct Func_Asm;


struct MOperand {
    Operand_Tag tag = ERRORTYPE;  // 操作数类型标识

    /**
     * 操作数的值
     * - 对于REG/VREG: 寄存器编号
     * - 对于IMM/SIMM: 立即数值
     * - 对于STACK: 相对于sp寄存器的偏移量，例如value=4表示[sp+4]
     */
    int32 value; 
    
    Shift_Tag s_tag = Nothing;  // 移位类型 (LSL, LSR, ASR等)
    uint8 s_value = 0;         // 移位量
    float fvalue;              // 浮点值（用于浮点立即数）
    const char* adr = NULL;    // 用于全局地址标识符
    
    // 构造函数
    MOperand() {}
    
    // 用于全局地址的构造函数
    MOperand(Operand_Tag tag, const char* adr) : tag(tag), adr(adr) {};
    
    // 用于数值类操作数的构造函数
    MOperand(Operand_Tag tag, int32 value) : tag(tag), value(value) {};
    
    // 带移位信息的构造函数
    MOperand(Operand_Tag tag, int32 value, Shift_Tag s_tag, uint8 s_value) : tag(tag), value(value), s_tag(s_tag), s_value(s_value) {};
    
    // 比较运算符重载，用于集合操作和排序
    bool operator<(const MOperand &b) const {
        if (tag != b.tag) return tag < b.tag;

        // 对于各类寄存器和立即数，比较其值
        if (tag == REG || tag == VREG || tag == IMM || tag == SREG ||tag == VSREG || tag == SIMM) {
            return value < b.value;
        }

        // 对于全局地址，比较地址字符串指针
        if (tag == ADR_GLOBAL) {
            return adr < b.adr;
        }

        assert(false);
        return false;
    }

    // 相等运算符重载，用于操作数比较
    bool operator==(const MOperand &b) const {
        if (tag != b.tag) return false;
        if (tag == REG || tag == VREG || tag == IMM || tag == SREG ||tag == VSREG || tag == SIMM) return value == b.value;
        if (tag == ADR_GLOBAL) return adr == b.adr;
        assert(false);
        return false;
    }

    // 使用-定义链相关方法
    void set_use_head(MI_Use *use, Func_Asm *func);       // 设置普通操作数使用链表头
    MI_Use *get_use_head(Func_Asm *func);                 // 获取普通操作数使用链表头
    void add_use(MI_Use *u, Func_Asm *func);              // 添加普通操作数使用
    MI *get_def_I(Func_Asm *func);                        // 获取定义此操作数的指令
    void set_def_I(MI *I, Func_Asm *func);                // 设置定义此操作数的指令
};

MOperand make_imm(int32 constant);
MOperand make_reg(uint8 reg);
MOperand make_vreg(int32 vreg_index);
MOperand make_ror_imm(int32 constant, Machine_Block *mb);


enum MI_Tag {
    MI_MOVE,      // 移动指令，对应ARM的mov/mvn
    MI_BINARY,    // 二元运算指令，如add, sub, mul, div等
    MI_CLZ,       // 计数前导零指令，count leading zeros
    MI_RETURN,    // 返回指令，对应ARM的bx lr
    MI_BRANCH,    // 分支跳转指令，如b, beq等条件跳转
    MI_COMPARE,   // 比较指令，对应ARM的cmp/cmn
    MI_FUNC_CALL, // 函数调用指令，对应ARM的bl
    MI_PUSH,      // 压栈指令，对应ARM的push
    MI_POP,       // 出栈指令，对应ARM的pop
    MI_LOAD,      // 加载指令，对应ARM的ldr
    MI_STORE,     // 存储指令，对应ARM的str
    MI_COMPLEXMUL,// 复杂乘法指令，如mla和mls
    // 浮点指令
    MI_VMOVE,     // 浮点移动指令，对应ARM的vmov
    MI_VBINARY,   // 浮点二元运算指令，如vadd, vsub等
    MI_VCOMPARE,  // 浮点比较指令，对应ARM的vcmp
    MI_VPUSH,     // 浮点压栈指令，对应ARM的vpush
    MI_VPOP,      // 浮点出栈指令，对应ARM的vpop
    MI_VLOAD,     // 浮点加载指令，对应ARM的vldr
    MI_VSTORE,    // 浮点存储指令，对应ARM的vstr
    MI_VCVT       // 浮点转换指令，对应ARM的vcvt
};


// Machine Instruction
struct MI {
    MI_Tag tag;                       // 指令类型标签
    MI *prev = NULL, *next = NULL;    // 前驱和后继指令指针，形成链表结构
    Machine_Block *mb = NULL;         // 指令所在的机器基本块
    Branch_Condition cond = NO_CONDITION; // 条件执行的条件码
    bool32 marked = false;            // 标记位，用于指令删除等操作
    bool32 update_flags = false;      // 是否更新条件标志位（对应ARM指令的S后缀）

    int32 n = -1;                     // 用于线性扫描寄存器分配算法

    MI(MI_Tag tag) : tag(tag) {}      // 构造函数
    MI(MI_Tag tag, Branch_Condition cond) : tag(tag), cond(cond) {} // 带条件的构造函数
    void erase_from_parent();         // 从父基本块中删除此指令
    void mark() { marked = true; }    // 标记此指令
};

// mov/mvn指令
struct MI_Move : MI {
    MOperand dst;                     // 目标操作数
    MOperand src;                     // 源操作数
    bool neg = false;                 // 是否取反，对应ARM的mvn指令

    MI_Move(MOperand dst, MOperand src) : MI(MI_MOVE), dst(dst), src(src) {};
};

// 用于比较两个MI_Move指针的比较器，用于在集合或映射中排序
struct MI_Move_Pointer_Cmp {
    bool operator()(const MI_Move *lhs, const MI_Move *rhs) const {
        if (!(lhs->dst == rhs->dst)) return (lhs->dst) < (rhs->dst);
        if (!(lhs->neg == rhs->neg)) return (lhs->neg) < (rhs->neg);
        return (lhs->src) < (rhs->src);
    }
};

// 计数前导零指令
struct MI_Clz : MI {
    MOperand dst;                     // 存储结果的操作数
    MOperand operand;                 // 要计数前导零的操作数

    MI_Clz() : MI(MI_CLZ) {};
};

// 二元操作
struct MI_Binary : MI {
    Binary_Op_Type op;       // 操作类型（加、减、乘等）
    MOperand dst;            // 目标操作数
    MOperand lhs, rhs;       // 左操作数和右操作数

	MI_Binary() : MI(MI_BINARY) {}
    MI_Binary(Binary_Op_Type op, MOperand dst, MOperand lhs, MOperand rhs) : 
        MI(MI_BINARY), op(op), dst(dst), lhs(lhs), rhs(rhs) {};
};

// 复合乘法指令
struct MI_ComplexMul : MI {
    ComplexMul_Op_Type op;   // 复杂乘法操作类型
    MOperand dst;            // 目标操作数
    MOperand lhs, rhs;       // 相乘的两个操作数
    MOperand extra;          // 额外操作数（如被加/减的值）

    MI_ComplexMul() : MI(MI_COMPLEXMUL) {}
    MI_ComplexMul(ComplexMul_Op_Type _op, MOperand _dst, MOperand _lhs, MOperand _rhs, MOperand _extra):
        MI(MI_COMPLEXMUL), op(_op), dst(_dst), lhs(_lhs), rhs(_rhs), extra(_extra) {}
};

// 比较指令
struct MI_Compare : MI {
    MOperand lhs, rhs;       // 被比较的两个操作数
    bool neg = false;        // 是否使用CMN（比较负值）而非CMP

    MI_Compare(MOperand lhs, MOperand rhs) : 
        MI(MI_COMPARE), lhs(lhs), rhs(rhs) {};
};

// 调用指令
struct MI_Func_Call : MI {
    const char *func_name;   // 被调用函数名
    int arg_count = 0;       // 参数数量
    int arg_stack_size = 0;  // 栈上参数所占空间大小

    MI_Func_Call(const char *func_name) : MI(MI_FUNC_CALL), func_name(func_name) {};
};

// 分支跳转指令
struct MI_Branch : MI {
    Machine_Block *true_target;  // 条件为真时跳转目标
    Machine_Block *false_target; // 条件为假时跳转目标（可为空）

    MI_Branch() : MI(MI_BRANCH) {};
    MI_Branch(Branch_Condition cond, Machine_Block *true_target, Machine_Block *false_target=NULL) : 
        MI(MI_BRANCH, cond), true_target(true_target), false_target(false_target) {};
};

// 返回指令
struct MI_Return : MI {
    MI_Return() : MI(MI_RETURN) {};
};

// Push指令
struct MI_Push : MI {
    Array<MOperand> operands;  // 要压栈的操作数数组
    MI_Push() : MI(MI_PUSH) {};
};

// Pop指令
struct MI_Pop : MI {
    Array<MOperand> operands;  // 接收出栈值的操作数数组
    MI_Pop() : MI(MI_POP) {};
};

// 内存访问类型
enum Mem_Tag {
    MEM_UNDEFINED,               // 未定义的内存操作
    MEM_ARRAY,                   // 数组访问
    MEM_LOAD_ARG,                // 加载函数参数
    MEM_LOAD_GLOBAL_REF,         // 加载全局引用
    MEM_LOAD_FROM_LITERAL_POOL,  // 从字面量池加载
    MEM_LOAD_SPILL,              // 从溢出区加载
    MEM_SAVE_SPILL,              // 保存到溢出区
    MEM_PREP_ARG                 // 准备参数
};

// Load指令
struct MI_Load : MI {
    Mem_Tag mem_tag = MEM_UNDEFINED;  // 内存操作类型
    MOperand reg;                      // 目标寄存器
    MOperand base;                     // 基址寄存器
	MOperand offset;                   // 偏移量
    MI_Load() : MI(MI_LOAD) {};
};

// Store指令
struct MI_Store : MI {
    Mem_Tag mem_tag = MEM_UNDEFINED;  // 内存操作类型
    MOperand reg;                      // 源寄存器
    MOperand base;                     // 基址寄存器
	MOperand offset;                   // 偏移量
    MI_Store() : MI(MI_STORE) {};
};

// 浮点Mov/mvn
struct MI_VMove : MI {
    MOperand dst;                // 目标操作数
    MOperand src;                // 源操作数
    bool both_float = false;     // 是否两端都是浮点数（vmov.f32）

    MI_VMove() : MI(MI_VMOVE){};
    MI_VMove(MOperand dst, MOperand src, bool both) : MI(MI_VMOVE), dst(dst), src(src) ,both_float(both){};

};

// 用于比较两个MI_VMove指针的比较器，用于在集合或映射中排序
struct MI_VMove_Pointer_Cmp
{
    bool operator()(const MI_VMove *lhs, const MI_VMove *rhs) const {
        if (!(lhs->dst == rhs->dst)) return (lhs->dst) < (rhs->dst);
        return (lhs->src) < (rhs->src);
    }
};

// 浮点二元操作指令
struct MI_VBinary : MI {
    Binary_Op_Type op;        // 操作类型（加、减、乘等）
    MOperand dst;             // 目标操作数
    MOperand lhs, rhs;        // 左操作数和右操作数
    int fneg=0;               // 是否对结果取负

    MI_VBinary() : MI(MI_VBINARY) {}
    MI_VBinary(Binary_Op_Type op, MOperand dst, MOperand lhs, MOperand rhs) : MI(MI_VBINARY), op(op), dst(dst), lhs(lhs), rhs(rhs) {};
};

// 浮点比较指令
struct MI_VCompare : MI {
    MOperand lhs, rhs;        // 被比较的两个浮点操作数
    bool neg = false;         // 是否取反比较结果

    MI_VCompare(MOperand lhs, MOperand rhs) : MI(MI_VCOMPARE), lhs(lhs), rhs(rhs) {}
};

// 浮点加载指令
struct MI_VLoad : MI {
    Mem_Tag mem_tag = MEM_UNDEFINED;  // 内存操作类型
    MOperand reg;                      // 目标浮点寄存器
    MOperand base;                     // 基址寄存器
    MOperand offset;                   // 偏移量
    MI_VLoad() : MI(MI_VLOAD) {};
};

// 浮点存储指令
struct MI_VStore : MI {
    Mem_Tag mem_tag = MEM_UNDEFINED;  // 内存操作类型
    MOperand reg;                      // 源浮点寄存器
    MOperand base;                     // 基址寄存器
	MOperand offset;                   // 偏移量
    MI_VStore() : MI(MI_VSTORE) {};
};

// 用于类型转换
enum Vcvt_Type {
    U32 = 0, // 无符号32位整数
    S32 = 1, // 有符号32位整数
    F32 = 2  // 浮点数
};

// 类型转换指令
struct MI_VCvt : MI {
    Vcvt_Type from_type;      // 源类型
    Vcvt_Type to_type;        // 目标类型
    MOperand dst;             // 目标操作数
    MOperand src;             // 源操作数
    MI_VCvt() : MI(MI_VCVT) {};
    MI_VCvt(Vcvt_Type from, Vcvt_Type to) : MI(MI_VCVT), from_type(from), to_type(to) {};
};

struct Machine_Block {
    int32 i = -1;                    // 基本块的唯一标识符
    MI *inst = NULL;                 // 基本块中的第一条指令
    MI *last_inst = NULL;            // 基本块中的最后一条指令
    MI *control_transfer_inst = NULL;// 控制流转移指令（如跳转、分支等）
    Array<Machine_Block *> preds;    // 前驱基本块列表
    Array<Machine_Block *> succs;    // 后继基本块列表
    Func_Asm *func = NULL;          // 所属函数

    // 用于基本块放置优化
    bool visited = false;            // 是否已被访问
    bool condified = false;          // 是否已被条件化处理

    LabelPtr label;                  // 基本块的标签

    uint32 loop_depth = -1;          // 循环嵌套深度
    int32 belongs_to_loop = -1;      // 所属循环的标识符

    void erase_marked_values();      // 删除被标记的值
    void push(MI *mi);
};

// 函数汇编
struct Func_Asm {
    int vreg_count = 0;              // 虚拟寄存器计数
    int index;                       // 函数索引
    int stack_size = 0;              // 栈帧大小
    bool has_return_value = true;    // 是否有返回值
    bool too_many_globals = false;   // 全局变量是否过多（最大流算法的工作区）
    int reg_needs_save = 0;          // 需要保存的寄存器数量
    int sreg_needs_save = 0;         // 需要保存的浮点寄存器数量
    int reg_spill_size = 0;          // 寄存器溢出区大小
    int sreg_spill_size = 0;         // 浮点寄存器溢出区大小

    string name;                     // 函数名
    Array<Machine_Block *> mbs;      // 函数中的所有基本块
    Array<const char *> global_value;// 全局变量列表
    Map<BasicBlockPtr, int> bb2idx;  // 基本块到索引的映射
    Map<int, BasicBlockPtr> idx2bb;  // 索引到基本块的映射

    // 使用-定义链相关映射
    map<pair<Operand_Tag, int32>, MI_Use *> use_head_map;    // 普通操作数使用链表头映射
    map<pair<Operand_Tag, int32>, MI *> def_I_map;           // 操作数定义指令映射

    Array<MI *> local_array_bases;   // 需要修复栈指针的局部数组基址
};

// 程序汇编
struct Program_Asm {
    Array<Func_Asm *> functions;     // 程序中的所有函数
};

// 机器指令Use
struct MI_Use {
    MI_Use *prev = nullptr, *next = nullptr;  // 使用链表的前后指针
    MOperand *val = nullptr;                  // 被使用的操作数
    MI *I = nullptr;                          // 使用该操作数的指令
    MOperand *user = nullptr;                 // 使用该操作数的位置
    MI_Use() {}                               // 默认构造函数
    MI_Use(MOperand *val, MOperand *user, MI *I): val(val), user(user), I(I) {}  // 带参数的构造函数
    void rm_use(Func_Asm *func);              // 从使用链中移除
};

// 生成操作数
MOperand make_operand(ValuePtr v, Machine_Block *mb, bool no_imm = false);

/**
 * 从Use链中移除指令对操作数的使用
 * @param I 要移除的指令
 * @param v 操作数
 * @param func 所属函数
 * @return 是否成功移除
 */
bool rm_instruction_use(MI* I, MOperand *v, Func_Asm *func);

// 在before前插入mi指令
void insert(MI *mi, MI *before);

// 检查立即数是否可以通过循环右移表示
bool can_be_imm_ror(int32 x);

// 检查立即数是否在12位范围内
bool can_be_imm12(int32 x);

/**
 * 生成加载常量的指令
 * @param vreg 目标虚拟寄存器
 * @param constant 要加载的常量值
 * @param mb 目标基本块
 * @param I 插入位置（可选）
 * @return 生成的加载指令
 */
MI_Load *emit_load_of_constant(MOperand vreg, int32 constant, Machine_Block *mb, MI *I=NULL);

// 生成mov指令
MI_Move *emit_move(MOperand dst, MOperand src, Machine_Block *mb = NULL);

// 生成函数的汇编代码
Func_Asm *emit_function_asm(Module func_IR, int i, vector<VariablePtr> &globalValues);

// 获取分支条件对应的汇编后缀字符串
const char *get_branch_suffix(Branch_Condition condition);

/**
 * 生成加载全局引用的指令
 * @param func_asm 函数汇编
 * @param v 全局变量
 * @param vreg 目标虚拟寄存器
 * @param mb 目标基本块
 */
void emit_load_of_global_ref(Func_Asm *func_asm, ValuePtr v, MOperand vreg,
                             Machine_Block *mb);
// 清除与函数生成相关的全局变量
void clear_function_related_variables();

// 生成汇编代码
Program_Asm *emit_asm(Module program_IR);

// 检查寄存器是否为被调用者保存寄存器
bool is_callee_save(uint8 reg);

// 检查寄存器是否为调用者保存寄存器
bool is_caller_save(uint8 reg);

// 构建操作数汇编代码
void build_operand(String_Builder *s, MOperand op);

// 构建函数的汇编代码
void build_function_asm(String_Builder *s, Func_Asm *func);

// 构建程序的汇编代码
void build_program_asm(String_Builder *s, Program_Asm *pro, vector<VariablePtr> & globalVariables);

// 输出操作数
void print_operand(MOperand op);

// 输出函数的汇编代码
void print_function_asm(Func_Asm *func);

// 获取全局变量的初始化指令序列
void get_init_sequence(VariablePtr globalarr,
                       vector<Pair<string, int>> &init_inst);

// 构建全局变量的汇编代码
void build_globals(String_Builder *s, vector<VariablePtr> &globals);

// 输出程序的汇编代码
void print_program_asm(Program_Asm *pro, vector<VariablePtr> & globalVariables);

// 输出基本块的汇编代码
void print_machine_block_asm(Machine_Block *mb);

void emit_load_of_later_arg(ValuePtr a, MOperand vreg, Machine_Block *mb,
                            int cum_offset);

// 反转分支条件
Branch_Condition invert_branch_cond(Branch_Condition cond);

#endif
