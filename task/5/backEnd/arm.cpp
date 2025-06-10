#include "arm.h"
#include "Module.h"
#include <cmath>
using namespace std;

int vreg_count;
int ori_func_stack;

static Map<ValuePtr, MOperand> value_map;
static Map<ValuePtr, MOperand> array_ptr_map;
static Map<ValuePtr, int> ptr_val_map;
static map<string, Binary_Op_Type> Binary_ir2asm;
static map<ValuePtr, ValuePtr> my_arr_base;
static map<ValuePtr, int> stack_val_map;
static map<pair<ValuePtr, ValuePtr>, MOperand> vv_div_map;
static map<pair<int, ValuePtr>, MOperand> cv_div_map;
static map<pair<ValuePtr, int>, MOperand> vc_div_map;
ValuePtr cur_arr_base;

MOperand make_imm(int32 constant) { return MOperand(IMM, constant); } // 创建立即数操作数
MOperand make_reg(uint8 reg) { return MOperand(REG, reg); } // 创建物理寄存器操作数
MOperand make_vreg(int32 vreg_index) { return MOperand(VREG, vreg_index); } // 创建虚拟寄存器操作数

// 创建可用于ARM指令的立即数操作数，如果常量无法通过循环右移表示，则加载到虚拟寄存器中
MOperand make_ror_imm(int32 constant, Machine_Block *mb) {
    if (!can_be_imm_ror(constant)) {
        auto vreg = make_vreg(vreg_count++);
        auto ldr = emit_load_of_constant(vreg, constant, mb);
        return vreg;
    } else {
        return make_imm(constant);
    }
}

// 获取栈上变量的地址
MOperand get_stack_val(ValuePtr v, Machine_Block *mb, Func_Asm *func_asm) {
    int val = 0;
    // 计算变量在栈中的偏移量
    if (ori_func_stack != 0) {
        val = ori_func_stack - stack_val_map[v];
    } else {
        val = func_asm->stack_size - stack_val_map[v];
    }

    auto nreg = make_vreg(vreg_count++);
    auto bi_I = new MI_Binary(BINARY_ADD, nreg, make_reg(sp), make_imm(val));
    // 如果偏移量超出立即数范围，先加载到寄存器
    if (!can_be_imm12(val)) {
        // 由于SSA限制不能使用nreg作为目标
        auto load_imm = emit_load_of_constant(make_vreg(vreg_count++), val, mb);
        bi_I->rhs = load_imm->reg;
    }
    mb->push((MI *)bi_I);
    return bi_I->dst;
}

// 在指定指令前插入新指令
void insert(MI *mi, MI *before) {
    // 处理插入到基本块开头的情况
    if (before->prev == NULL) {
        before->mb->inst = mi;
    } else {
        before->prev->next = mi;
    }
    // 更新指令链表指针
    mi->mb = before->mb;
    mi->prev = before->prev;
    mi->next = before;
    before->prev = mi;
}

Branch_Condition binary_op_to_branch_cond(Binary_Op_Type op_type) {
    switch (op_type) {
        case BINARY_LESS_THAN: {
            return LESS_THAN;
        } break;
        case BINARY_GREATER_THAN: {
            return GREATER_THAN;
        } break;
        case BINARY_LESS_THAN_OR_EQUAL_TO: {
            return LESS_THAN_OR_EQUAL;
        } break;
        case BINARY_GREATER_THAN_OR_EQUAL_TO: {
            return GREATER_THAN_OR_EQUAL;
        } break;
        case BINARY_NOT_EQUAL_TO: {
            return NOT_EQUAL;
        } break;
        case BINARY_EQUAL_TO: {
            return EQUAL;
        } break;
        case FLOAT_GE: {
            return float_ge;
        } break;
        default: {
            exit(31);
            report_error("Internal Compiler Error: converting unknown binary op "
                        "type to branch type.");
        }
    }
    return NO_CONDITION;
}

// 将IR中的值转换为机器操作数
MOperand make_operand(ValuePtr v, Machine_Block *mb, bool no_imm) {
    // 检查该值是否已有对应的操作数
    auto opr = value_map.find(v);
    if (opr != value_map.end()) {
        return opr->second;
    }

    int tp = -1;
    if (v->isConst)
        tp = 0;      // 常量
    else if (v->isReg)
        tp = 1;      // 寄存器
    else if (v->isVoid)
        tp = 2;      // 空值
    else {
        if (v->type->isInt())
            tp = 3;  // 整数
        else if (v->type->isArr())
            tp = 4;  // 数组
        else if (v->type->isPtr())
            tp = 5;  // 指针
        else if (v->type->isBool())
            tp = 6;  // 布尔值
    }

    switch (tp) {
        // 常量处理
        case 0: {
            int32 constant = dynamic_cast<Const *>(v.get())->intVal;
            auto imm = make_imm(constant);
            // 检查常量是否可以表示为ARM的立即数格式
            if (!can_be_imm_ror(constant)) {
                no_imm = true;  // 不能表示为立即数，强制加载到寄存器
            }
            if (no_imm) {
                // 创建一个虚拟寄存器并加载常量值
                auto vreg = make_vreg(vreg_count++);
                auto ldr = emit_load_of_constant(vreg, constant, mb);
                return vreg;
            } else {
                return imm;  // 直接返回立即数
            }
        } break;
        // 整数处理
        case 3: {
            int32 constant = dynamic_cast<Int *>(v.get())->intVal;
            auto imm = make_imm(constant);
            if (no_imm) {
                // 创建虚拟寄存器并加载整数值
                auto vreg = make_vreg(vreg_count++);
                MI_Load *ldr = emit_load_of_constant(vreg, constant, mb);
                return vreg;
            } else {
                return imm;  // 直接返回立即数
            }
        } break;
        // 数组处理
        case 4: {
            int32 constant = 0;
            auto imm = make_imm(constant);
            if (no_imm) {
                // 创建虚拟寄存器表示数组
                auto vreg = make_vreg(vreg_count++);
                value_map[v] = vreg;  // 记录值到操作数的映射
                return vreg;
            } else {
                // 数组不应该直接返回0
                assert(false && "return zero in make_operand:arr");
                return imm;
            }
        } break;
        // 指针处理
        case 5: {
            int32 constant = 0;
            auto imm = make_imm(constant);
            if (no_imm) {
                // 创建虚拟寄存器表示指针
                auto vreg = make_vreg(vreg_count++);
                value_map[v] = vreg;  // 记录值到操作数的映射
                return vreg;
            } else {
                // 指针不应该直接返回0
                assert(false && "return zero in make_operand:ptr");
                return imm;
            }
        } break;

        // 其他类型的处理
        default: {
            // 创建新的虚拟寄存器
            MOperand new_vreg;
            new_vreg = make_vreg(vreg_count++);
            value_map[v] = new_vreg;  // 记录值到操作数的映射
            return new_vreg;
        }
    }
}

// 清除与函数生成相关的全局变量
void clear_function_related_variables() {
    value_map.clear();      // 清除IR值到操作数的映射
    array_ptr_map.clear();  // 清除数组指针映射
    ptr_val_map.clear();    // 清除指针值映射
    my_arr_base.clear();    // 清除数组基址映射
    stack_val_map.clear();  // 清除栈变量映射
    vv_div_map.clear();     // 清除变量对变量除法结果缓存
    cv_div_map.clear();     // 清除常量对变量除法结果缓存
    vc_div_map.clear();     // 清除变量对常量除法结果缓存
}

// 获取分支条件对应的汇编后缀字符串
const char *get_branch_suffix(Branch_Condition condition) {
    switch (condition) {
    case NO_CONDITION: {
        return "";
    } break;
    case LESS_THAN: {
        return "lt";
    } break;
    case GREATER_THAN: {
        return "gt";
    } break;
    case LESS_THAN_OR_EQUAL: {
        return "le";
    } break;
    case GREATER_THAN_OR_EQUAL: {
        return "ge";
    } break;
    case NOT_EQUAL: {
        return "ne";
    } break;
    case EQUAL: {
        return "eq";
    } break;
    default: {
        assert(false);
        return "";
    }
    }
}

// 反转分支条件
Branch_Condition invert_branch_cond(Branch_Condition cond) {
    if (cond == NO_CONDITION || cond >= 7)
        return cond;
    if (cond >= 1 && cond <= 3) {
        return (Branch_Condition)(cond + 3);
    } else if (cond >= 4 && cond <= 6) {
        return (Branch_Condition)(cond - 3);
    }
    assert(false);
    return NO_CONDITION;
}

// 判断常量是否可通过循环右移编码为ARM指令的立即数
bool can_be_imm_ror(int32 x) {
    uint32 v = x;
    for (int r = 0; r < 31; r += 2) {
        if ((v & 0xff) == v) {
            return true;
        }

        v = (v >> 2) | (v << 30);
    }
    return false;
}

// 获取全局数组或整型变量的初始化指令序列
void get_init_sequence(VariablePtr globalarr,
                       vector<Pair<string, int>> &init_inst) {
    if (globalarr->type->isInt()) {
        int val_use = dynamic_cast<Int *>(globalarr.get())->intVal;
        if (val_use) {
            init_inst.emplace_back(make_pair(".word", val_use));
        } else {
            if (init_inst.empty() || init_inst.back().first == ".word") {
                init_inst.emplace_back(make_pair(".zero", 0));
            }
            init_inst.back().second += 4;
        }
        return;
    }
    if (globalarr->zero()) {
        if (init_inst.empty() || init_inst.back().first == ".word") {
            init_inst.emplace_back(make_pair(".zero", 0));
        }
        int zero_size =
            dynamic_cast<ArrType *>((globalarr->type).get())->getSize();
        init_inst.back().second += zero_size * 4;
        return;
    } else {
        auto arr_use = dynamic_cast<Arr *>(globalarr.get());
        int i = 0;
        for (; i < arr_use->inner.size(); i++) {
            get_init_sequence(arr_use->inner[i], init_inst);
        }
        if (arr_use->getElementType()->isArr()) {
            for (; i < arr_use->getElementLength(); i++) {
                get_init_sequence(arr_use->inner[i], init_inst);
            }
        } else if (arr_use->getElementType()->isInt()) {
            if (i != arr_use->getElementLength()) {
                if (init_inst.empty() || init_inst.back().first == ".word") {
                    init_inst.emplace_back(make_pair(".zero", 0));
                }
                init_inst.back().second +=
                    (arr_use->getElementLength() - i) * 4;
            }
        }
    }
}

/**
 * 生成全局变量的ARM汇编数据段和bss段
 * 遍历所有全局变量，根据变量类型生成对应的汇编声明和初始化内容。
 * 整型变量生成.data段的.word指令，数组变量根据是否全为零分别生成.data段初始化或.bss段未初始化空间。
 * 
 * @param s 字符串构建器，用于拼接最终的汇编文本
 * @param globals 全局变量列表，包含所有需要生成汇编的全局变量
 */
void build_globals(String_Builder *s, vector<VariablePtr> &globals) {
    if (globals.size() == 0)
        return;
    s->append(".data\n.align 2\n");
    for (int i = 0; i < globals.size(); i++) {
        if (globals[i]->type->isInt()) {
            s->append("%s:\n", globals[i]->name.c_str());
            s->append("    ");
            int32 val = dynamic_cast<Int *>(globals[i].get())->intVal;
            s->append(".word %d\n", val);
        } else if (globals[i]->type->isArr()) {
            vector<Pair<string, int>> init_inst;
            auto globalarr = dynamic_cast<Arr *>(globals[i].get());
            if (globalarr->zero()) {
                continue;
            } else {
                get_init_sequence(globals[i], init_inst);
                s->append("%s:\n", globals[i]->name.c_str());
                for (int j = 0; j < init_inst.size(); j++) {
                    s->append("    ");
                    s->append("%s  %d\n", init_inst[j].first.c_str(),
                              init_inst[j].second);
                }
            }
        } else {
            printf("globals error type\n");
        }
    }
    s->append(".bss\n");
    s->append(".align 2\n");
    for (int i = 0; i < globals.size(); i++) {
        if (globals[i]->type->isArr()) {
            auto globalarr = dynamic_cast<Arr *>(globals[i].get());
            if (globalarr->zero()) {
                int zero_size =
                    dynamic_cast<ArrType *>((globalarr->type).get())->getSize();
                zero_size *= 4;
                s->append("%s:\n", globals[i]->name.c_str());
                s->append("    .space %d\n", zero_size);
                s->append("    .type %s, %%object\n", globals[i]->name.c_str());
                s->append("    .size %s, %d\n", globals[i]->name.c_str(),
                          zero_size);
            }
        }
    }
}

// 判断寄存器是否为被调用者保存寄存器
bool is_callee_save(uint8 reg) { return (reg >= r4 && reg <= r11) || reg == lr; }

// 判断寄存器是否为调用者保存的寄存器
bool is_caller_save(uint8 reg) { return (reg >= r0 && reg <= r3) || reg == r12; }

// 判断常量是否可作为ARM指令的12位立即数
bool can_be_imm12(int32 x) { return (x >= -4095) && (x <= 4095); }

// 移除指令在整数操作数使用链表中的使用记录
bool rm_instruction_use(MI *I, MOperand *v, Func_Asm *func) {
    auto use = v->get_use_head(func);
    while (use != nullptr) {
        if (use->I == I) {
            use->rm_use(func);
            return true;
        }
        use = use->next;
    }
    return false;
}

// 从基本块中移除当前指令
void MI::erase_from_parent() {
    if (prev) {
        prev->next = next;
    }
    if (next) {
        next->prev = prev;
    }
    if (mb->inst == this) {
        mb->inst = next;
    }
    if (mb->last_inst == this) {
        mb->last_inst == prev;
    }
    if (mb->control_transfer_inst == this) {
        mb->control_transfer_inst = NULL;
    }
    next = prev = NULL;
    mb = NULL;
}

// 基本块中移除已标记的指令
void Machine_Block::erase_marked_values() {
    static Array<MI *> unmarked_values;
    unmarked_values.len = 0;

    for (MI *I = inst; I; I = I->next) {
        if (!I->marked) {
            unmarked_values.push(I);
        }
    }

    if (unmarked_values.len == 0) {
        inst = last_inst = control_transfer_inst = 0;
        succs = {};
    } else {
        inst = unmarked_values[0];
        last_inst = control_transfer_inst =
            unmarked_values[unmarked_values.len - 1];

        inst->prev = NULL;
        last_inst->next = NULL;

        unmarked_values.push(NULL);
        for (int i = 0; i < unmarked_values.len - 1; i++) {
            auto p = unmarked_values[i];
            auto n = unmarked_values[i + 1];
            p->next = n;
            if (n)
                n->prev = p;
        }
    }
}

// 将指令添加到基本块末尾
void Machine_Block::push(MI *mi) {
    mi->mb = this;
    if (last_inst) {
        last_inst->next = mi;
        mi->prev = last_inst;
    } else {
        inst = mi;
    }
    last_inst = mi;
}

// MOperand使用-定义链相关函数
// -----------------------------------------------

// 设置当前整数操作数的使用链表头节点
void MOperand::set_use_head(MI_Use *use, Func_Asm *func) {
    auto p = pair<Operand_Tag, int32>(tag, value);
    func->use_head_map[p] = use;
}

// 获取当前整数操作数的使用链表头节点
MI_Use *MOperand::get_use_head(Func_Asm *func) {
    if (tag != VREG && tag != REG)
        return nullptr;
    auto p = pair<Operand_Tag, int32>(tag, value);
    return func->use_head_map[p];
}

// 将使用节点添加到当前操作数的使用链表头部
void MOperand::add_use(MI_Use *u, Func_Asm *func) {
    u->prev = nullptr;
    auto use_head = get_use_head(func);
    if (use_head) {
        use_head->prev = u;
    }
    u->next = use_head;
    set_use_head(u, func);
}

// 获取当前操作数的定义指令
MI *MOperand::get_def_I(Func_Asm *func) {
    auto p = pair<Operand_Tag, int32>(tag, value);
    return func->def_I_map[p];
}

// 设置当前操作数的定义指令
void MOperand::set_def_I(MI *I, Func_Asm *func) {
    auto p = pair<Operand_Tag, int32>(tag, value);
    func->def_I_map[p] = I;
}

// 从操作数使用链表中移除当前节点
void MI_Use::rm_use(Func_Asm *func) {
    if (prev == nullptr && next == nullptr) {
        val->set_use_head(nullptr, func);
    } else if (prev == nullptr && next != nullptr) {
        val->set_use_head(next, func);
        next->prev = nullptr;
    } else if (prev != nullptr && next == nullptr) {
        prev->next = nullptr;
    } else if (prev != nullptr && next != nullptr) {
        prev->next = next;
        next->prev = prev;
    }
}

// __________________________________task5 begin_______________________________________

// 生成mov指令
MI_Move *emit_move(MOperand dst, MOperand src, Machine_Block *mb) {
    // todo
}

// 生成二元运算指令
void emit_Binary(InstructionPtr I, Machine_Block *mb) {
    auto bi_I = dynamic_cast<BinaryInstruction *>(I.get());
    string myop;
    myop += bi_I->op;
    auto op_type = Binary_ir2asm[myop];  // 查找对应的二元操作类型
    
    // 处理取模运算(%)，ARM没有直接的取模指令，需要特殊处理
    if (op_type == BINARY_MOD) {
        // todo
    } 
    // 处理一元非运算(!x)，转化为1-x
    else if (op_type == UNARY_XOR) {
        // todo
    } 
    // 处理其他二元操作
    else {
        // todo
    }
}

// 生成比较指令
void emit_Icmp(InstructionPtr I, Machine_Block *mb) {
    // todo
}

// 生成分支指令
void emit_Branch(Func_Asm *func_asm, InstructionPtr I, Machine_Block *mb) {
    // todo
}

// 生成返回指令
void emit_Return(Func_Asm *func_asm, InstructionPtr I, Machine_Block *mb) {
    // todo
}

// __________________________________task5 end_________________________________________

// 生成Load指令，将立即数值加载到虚拟寄存器中
MI_Load *emit_load_of_constant(MOperand vreg, int32 constant, Machine_Block *mb,
                               MI *I) {
    auto ldr = new MI_Load;
    ldr->mem_tag = MEM_LOAD_FROM_LITERAL_POOL;  // 标记为从字面量池加载
    ldr->reg = vreg;  // 设置目标寄存器
    ldr->base = make_imm(constant);  // 设置要加载的常量值
    if (mb) {
        if (I) {
            // 如果指定了插入位置，在该位置前插入
            (ldr, I);
        } else if (mb->control_transfer_inst) {
            // 如果基本块有控制转移指令，在该指令前插入
            insert(ldr, mb->control_transfer_inst);
        } else {
            // 否则添加到基本块末尾
            mb->push(ldr);
        }
    }
    return ldr;
}

// 生成指令从栈上加载函数参数（超过寄存器传递限制的参数）
void emit_load_of_later_arg(ValuePtr a, MOperand vreg, Machine_Block *mb,
                            int cum_offset) {
    auto offset_relative_to_fp = cum_offset;

    auto ldr = new MI_Load;
    ldr->mem_tag = MEM_LOAD_ARG;
    ldr->reg = vreg;
    ldr->base = make_reg(sp);
    ldr->offset = make_imm(offset_relative_to_fp);

    // 在控制转移指令前插入加载指令，或者添加到基本块末尾
    if (mb->control_transfer_inst) {
        insert(ldr, mb->control_transfer_inst);
    } else {
        mb->push(ldr);
    }
}

// 生成加载全局变量地址的指令
void emit_load_of_global_ref(Func_Asm *func_asm, ValuePtr v, MOperand vreg,
                             Machine_Block *mb) {
    const char *addr = new char[1024];       // 为变量名分配足够的内存空间
    addr = v->name.c_str();                  // 获取变量名
    MOperand adr_global(ADR_GLOBAL, addr);   // 创建全局地址操作数
    auto ldr = new MI_Load;                  // 创建新的加载指令
    ldr->reg = vreg;                         // 设置目标寄存器
    ldr->base = adr_global;                  // 设置加载源为全局地址
    ldr->mem_tag = MEM_LOAD_GLOBAL_REF;      // 标记为加载全局引用
    mb->push((MI *)ldr);                     // 将指令添加到基本块中
}

// 生成Gep指令
void emit_Gep(Func_Asm *func_asm, InstructionPtr I, Machine_Block *mb, vector<VariablePtr> &globalValues) {
    // 获取GetElementPtr指令指针，用于计算数组元素或结构体成员的地址
    auto ptr_I = dynamic_cast<GetElementPtrInstruction *>(I.get());
    auto sname = ptr_I->from->name;
    int isarrele = 0;  // 标记是否为数组元素访问
    // 为目标寄存器创建一个虚拟寄存器并记录在映射表中
    value_map[ptr_I->reg] = make_vreg(vreg_count++);

    // 处理数组基址 - 如果源操作数名称中不包含"arrayidx"
    if (ptr_I->from->name.find("arrayidx") ==
        ptr_I->from->name.npos) {
        // 更新数组基址映射关系
        if (my_arr_base.find(ptr_I->from) == my_arr_base.end())
            my_arr_base[ptr_I->reg] = ptr_I->from;  // 新的数组基址
        else
            my_arr_base[ptr_I->reg] = my_arr_base[ptr_I->from];  // 继承基址
        
        // 检查是否为全局变量
        int flag = 0;
        for (auto x : globalValues) {
            if (ptr_I->from == x) {
                flag = 1;
                break;
            }
        }
        // 如果是全局变量，加载其地址到虚拟寄存器
        if (flag) {
            auto nvreg = make_vreg(vreg_count++);
            emit_load_of_global_ref(func_asm, ptr_I->from, nvreg,
                                    mb);
            value_map[ptr_I->from] = nvreg;
        }
    } else {
        // 数组索引操作，继承基址
        my_arr_base[ptr_I->reg] = my_arr_base[ptr_I->from];
    }

    // 检查是否为数组初始化元素
    if (ptr_I->reg->name.find("arrayinit") !=
        ptr_I->reg->name.npos) {
        isarrele = 1;
    }

    // 初始化用于计算偏移量的变量
    TypePtr cur_arr = NULL;  // 当前处理的数组类型
    auto reg4 = make_imm(0);  // 初始偏移寄存器值为0
    int cff = 0;  // 是否有常量偏移标志
    int offset = 0;  // 累积的常量偏移量
    int ff = 0;  // 标记是否需要动态计算偏移量
    
    // 检查源操作数是否有已知的常量偏移量
    if (ptr_val_map.find(ptr_I->from) != ptr_val_map.end()) {
        offset = ptr_val_map.find(ptr_I->from)->second;
        cff = 1;
    }

    // 检查是否为数组初始化且有映射值
    if (value_map.find(ptr_I->from) != value_map.end() &&
        ptr_I->from->name.find("arrayinit") !=
            ptr_I->from->name.npos &&
        !cff) {
        ff = 1;  // 需要动态计算
        reg4 = value_map[ptr_I->from];  // 使用已映射的寄存器值
    }

    // 为多维数组索引创建临时存储
    const int max_dims = 40;  // 支持的最大维度数
    MOperand mul_regs[max_dims + 1];  // 存储每维乘法结果的寄存器
    vector<MI *> mul_instr(max_dims + 1, nullptr);  // 存储乘法指令

    // 检查索引是否包含变量，如果有则需要动态计算
    int cnt = 0;
    for (auto x : ptr_I->index) {
        if (!x->isConst)
            ff = 1;  // 有非常量索引，需要动态计算
    }
    
    // 处理数组元素访问
    if (isarrele) {
        for (int i = 0; i < ptr_I->index.size(); i++) {
            cnt++;
            auto x = ptr_I->index[i];
            int const_val = 0;

            // 获取常量索引值
            if (x->isConst)
                const_val = dynamic_cast<Const *>(x.get())->intVal;
            else {
                ff = 1;  // 非常量索引，需要动态计算
            }

            // 根据维度确定此维度的大小
            int const_size = 1;
            if (i == 0) {
                // 第一维特殊处理 - 检查类型是指针还是数组
                if (ptr_I->from->type->ID == PtrID) {
                    auto tmp = dynamic_cast<PtrType *>(
                        ptr_I->from->type.get());
                    if (tmp->inner->isArr())
                        const_size = dynamic_cast<ArrType *>(
                                        tmp->inner.get())
                                        ->getSize();
                } else if (ptr_I->from->type->ID == IntID ||
                        ptr_I->from->type->ID == FLoatID) {
                    const_size = 1;  // 基本类型大小为1个元素
                } else {
                    assert(false && "can not handle array type");
                }
            }
            if (i == 1) {
                // 第二维 - 获取第一维的内部类型
                assert(ptr_I->from->type->ID == ArrID);
                auto tmp = dynamic_cast<ArrType *>(
                    ptr_I->from->type.get());
                if (tmp->inner->isArr())
                    const_size =
                        dynamic_cast<ArrType *>(tmp->inner.get())
                            ->getSize();
                cur_arr = tmp->inner;  // 更新当前处理的数组类型
            } else if (i > 1) {
                // 更高维度 - 继续深入数组类型
                assert(cur_arr->ID == ArrID);
                auto nc = dynamic_cast<ArrType *>(cur_arr.get());
                if (nc->inner->isArr())
                    const_size =
                        dynamic_cast<ArrType *>(nc->inner.get())
                            ->getSize();
                cur_arr = nc->inner;  // 更新当前处理的数组类型
            }
            
            if (!ff)
                // 累加常量偏移量 = 维度大小 * 索引值
                offset += const_size * const_val;
            else {
                // 需要动态计算偏移量
                if (x != ptr_I->index.back() || const_size != 1) {
                    // 创建表示维度大小的整数常量
                    ValuePtr imptr = ValuePtr(
                        new Int("asdf", false, false, const_size));
                    // 创建乘法指令：索引 * 维度大小
                    auto bi_mul = new MI_Binary;
                    bi_mul->op = BINARY_MULTIPLY;
                    bi_mul->lhs = make_operand(x, mb, true);
                    bi_mul->rhs = make_operand(imptr, mb, true);
                    assert(cnt > 0 && cnt < max_dims);
                    mul_regs[cnt] = make_vreg(vreg_count++);
                    bi_mul->dst = mul_regs[cnt];
                    // 保存乘法指令，稍后添加到基本块
                    mul_instr[cnt] = bi_mul;
                } else {
                    // 最后一维且大小为1，直接使用索引值
                    assert(cnt > 0 && cnt < max_dims);
                    mul_regs[cnt] = make_operand(x, mb, true);
                }
            }
        }
    } else {
        // 处理非数组元素访问（如指针算术）
        for (int i = 1; i < ptr_I->index.size(); i++) {
            auto x = ptr_I->index[i];
            int const_val = 0;
            cnt++;

            // 获取常量索引值
            if (x->isConst)
                const_val = dynamic_cast<Const *>(x.get())->intVal;
            else {
                ff = 1;  // 非常量索引，需要动态计算
            }

            // 确定当前维度大小
            int const_size = 1;
            if (i == 1) {
                // 第二维 - 获取第一维的内部类型
                auto tmp = dynamic_cast<ArrType *>(
                    ptr_I->from->type.get());
                if (tmp->inner->isArr())
                    const_size =
                        dynamic_cast<ArrType *>(tmp->inner.get())
                            ->getSize();
                cur_arr = tmp->inner;  // 更新当前处理的数组类型
            } else {
                // 更高维度 - 继续深入数组类型
                auto nc = dynamic_cast<ArrType *>(cur_arr.get());
                if (nc->inner->isArr())
                    const_size =
                        dynamic_cast<ArrType *>(nc->inner.get())
                            ->getSize();
                cur_arr = nc->inner;  // 更新当前处理的数组类型
            }
            
            if (!ff)
                // 累加常量偏移量
                offset += const_size * const_val;
            else {
                // 需要动态计算偏移量
                if (x != ptr_I->index.back()) {
                    // 创建表示维度大小的整数常量
                    ValuePtr imptr = ValuePtr(
                        new Int("asdf", false, false, const_size));
                    // 创建乘法指令：索引 * 维度大小
                    auto bi_mul = new MI_Binary;
                    bi_mul->op = BINARY_MULTIPLY;
                    bi_mul->lhs = make_operand(x, mb, true);
                    bi_mul->rhs = make_operand(imptr, mb, true);
                    assert(cnt > 0 && cnt < max_dims);
                    mul_regs[cnt] = make_vreg(vreg_count++);
                    bi_mul->dst = mul_regs[cnt];
                    // 保存乘法指令，稍后添加到基本块
                    mul_instr[cnt] = bi_mul;
                } else {
                    // 最后一维，直接使用索引值
                    assert(cnt > 0 && cnt < max_dims);
                    mul_regs[cnt] = make_operand(x, mb, true);
                }
            }
        }
    }
    
    // 根据是否需要动态计算偏移量，生成不同的指令
    if (!ff) {
        // 使用常量偏移量
        auto myimm = make_imm(offset);
        auto curreg = make_operand(ptr_I->reg, mb, true);
        curreg.value = offset;  // 设置偏移值
        ptr_val_map[ptr_I->reg] = offset;  // 记录在映射表中
    } else {
        // 动态计算偏移量
        if (cnt == 1) {
            // 单维索引情况
            // 先添加乘法指令（如果有）
            if (mul_instr[cnt] != nullptr) {
                mb->push(mul_instr[cnt]);
            }
            // 创建加法指令：基址 + 偏移量
            auto bi_add = new MI_Binary(
                BINARY_ADD, make_operand(ptr_I->reg, mb, true),
                mul_regs[1], reg4);
            bi_add->dst = make_operand(ptr_I->reg, mb, true);
            mb->push((MI *)bi_add);
        } else if (cnt > 1) {
            // 多维索引情况，需要累加每一维的偏移量
            assert(cnt < max_dims);
            int i;
            // 从第一维开始累加
            auto last_reg = mul_regs[1];
            if (mul_instr[1] != nullptr) {
                mb->push(mul_instr[1]);
            }
            // 依次处理每一维的偏移量
            for (i = 1; i < cnt; i++) {
                if (mul_instr[i + 1] != nullptr) {
                    mb->push(mul_instr[i + 1]);
                }

                // 创建目标寄存器
                MOperand dst;
                if (i < cnt - 1)  // 中间结果放入临时寄存器
                    dst = make_vreg(vreg_count++);
                else if (i == cnt - 1)  // 最终结果放入目标寄存器
                    dst = make_operand(ptr_I->reg, mb, true);
                    
                // 创建加法指令：前面维度的累积值 + 当前维度的偏移量
                auto bi_add = new MI_Binary;
                bi_add->op = BINARY_ADD;
                bi_add->lhs = last_reg;  // 前面维度的累积值
                bi_add->rhs = mul_regs[i + 1];  // 当前维度的偏移量
                bi_add->dst = dst;  // 结果目标
                mb->push((MI *)bi_add);
                last_reg = bi_add->dst;  // 更新累积值
            }
        }
    }
}

// 生成函数调用指令
void emit_Call(Func_Asm *func_asm, InstructionPtr I, Machine_Block *mb){
    // 处理函数调用指令
    auto func_I = dynamic_cast<CallInstruction *>(I.get());
    auto func_name = func_I->func->name;
    
    // 特殊处理memset库函数调用
    if (func_name.size() > 15 &&
        func_name.substr(5, 6) == "memset") {
        auto cut_name = func_name.substr(5, 6);
        auto call = new MI_Func_Call("memset");
        call->arg_count = func_I->func->formArguments.size() - 1;
        
        // 准备memset的参数，放入r0-r2寄存器
        for (int i = 0; i < func_I->func->formArguments.size() - 1;i++) {
            auto arg_value = func_I->argv[i];

            MOperand arg_operand;
            arg_operand = make_reg(i + r0);  // 使用r0-r2物理寄存器
            emit_move(arg_operand, make_operand(arg_value, mb), mb);
        }

        mb->push((MI *)call);  // 添加函数调用指令

        ValuePtr ret = func_I->func->retVal;

        // 处理返回值(如果有)
        if (!ret->isVoid) {
            auto result = make_operand(func_I->reg, mb, true);
            emit_move(result, make_reg(r0), mb);  // 从r0获取返回值
        }
        return;
    }
    
    // 处理普通函数调用
    auto call = new MI_Func_Call(func_I->func->name.c_str());
    call->arg_count = func_I->func->formArguments.size();
    int myff = 0;  // 标记调用是否有效
    int offset = 0;  // 栈上参数的偏移量
    
    // 处理每个参数
    for (int i = 0; i < func_I->func->formArguments.size(); i++) {
        auto arg_value = func_I->argv[i];
        MOperand arg_operand;
        
        if (i < 4) {
            // 前4个参数通过r0-r3寄存器传递
            if (stack_val_map.find(arg_value) !=
                stack_val_map.end()) {
                // 参数在栈上，需要先加载
                arg_operand = make_reg(i + r0);
                auto ldr = new MI_Load;
                ldr->reg = make_vreg(vreg_count++);
                ldr->base = get_stack_val(arg_value, mb, func_asm);
                mb->push((MI *)ldr);
                emit_move(arg_operand, ldr->reg, mb);
            } else {
                // 参数在寄存器中
                arg_operand = make_reg(i + r0);
                auto mydes = make_operand(arg_value, mb, false);
                
                // 处理数组指针参数
                if (array_ptr_map.find(arg_value) !=
                    array_ptr_map.end()) {
                    mydes = array_ptr_map[arg_value];
                } else if (my_arr_base.find(arg_value) !=
                        my_arr_base.end()) {
                    // 处理数组基址+偏移量
                    auto arr_base =
                        my_arr_base.find(arg_value)->second;
                    MOperand my_offset;
                    int arrflag = 0;
                    
                    if (ptr_val_map.find(arg_value) ==
                        ptr_val_map.end()) {
                        // 动态偏移量，将索引左移2位(乘以4)
                        my_offset = make_vreg(vreg_count++);
                        auto bi_I = new MI_Binary;
                        bi_I->op = BINARY_LSL;
                        bi_I->lhs =
                            make_operand(arg_value, mb, true);
                        bi_I->rhs = make_imm(2);
                        bi_I->dst = my_offset;
                        mb->push((MI *)bi_I);
                        arrflag = 1;
                    } else {
                        // 常量偏移量，直接乘以4
                        my_offset = make_ror_imm(
                            ptr_val_map[arg_value] * 4, mb);
                    }

                    // 计算数组元素地址：基址 + 偏移量
                    mydes = make_vreg(vreg_count++);
                    auto bi_I = new MI_Binary(
                        BINARY_ADD, mydes,
                        make_operand(arr_base, mb, true),
                        my_offset);
                    mb->push((MI *)bi_I);

                    array_ptr_map[arg_value] = mydes;  // 缓存计算结果
                }
                emit_move(arg_operand, mydes, mb);  // 移动到参数寄存器
            }
        }
        else {
            // 超过4个参数使用栈传递
            if (ptr_val_map.find(arg_value) != ptr_val_map.end()) {
                // 处理数组元素指针
                cur_arr_base = my_arr_base[arg_value];
                auto opr = ptr_val_map.find(arg_value);
                
                // 计算数组元素地址：基址 + 偏移*4
                auto bi_add = new MI_Binary;
                bi_add->op = BINARY_ADD;
                bi_add->lhs = make_operand(cur_arr_base, mb);
                bi_add->rhs = make_ror_imm(opr->second * 4, mb);
                bi_add->dst = make_vreg(vreg_count++);
                mb->push((MI *)bi_add);
                
                // 存储到栈上对应位置
                auto str = new MI_Store();
                str->mem_tag = MEM_PREP_ARG;
                str->reg = bi_add->dst;
                str->base = make_reg(sp);
                str->offset = make_ror_imm(offset, mb);
                mb->push((MI *)str);
                offset += 4;  // 更新栈偏移
            }
            // 处理普通参数
            else {
                // 将参数值存储到栈上
                auto str = new MI_Store;
                str->mem_tag = MEM_PREP_ARG;
                str->reg = make_operand(arg_value, mb, true);
                str->base = make_reg(sp);
                str->offset = make_ror_imm(offset, mb);
                offset += 4;  // 更新栈偏移
                mb->push((MI *)str);
            }
        }
    }
    
    // 栈大小对齐到8字节边界
    if (offset % 8 != 0) {
        offset += 8 - (offset % 8);
    }
    call->arg_stack_size = offset;  // 记录栈上参数的总大小
    
    if (!myff)
        mb->push((MI *)call);  // 添加函数调用指令

    ValuePtr ret = func_I->func->retVal;

    // 处理返回值(如果有)
    if (!ret->isVoid) {
        auto result = make_operand(func_I->reg, mb, true);
        emit_move(result, make_reg(r0), mb);  // 从r0获取返回值
    }
}

// 生成栈内存申请指令   
void emit_Alloca(Func_Asm *func_asm, InstructionPtr I, Machine_Block *mb){     
    // 处理栈内存分配指令
    auto alloc = dynamic_cast<AllocaInstruction *>(I.get());
    auto tmp = dynamic_cast<Variable *>(alloc->des.get());
    int tp = tmp->type->getID();  // 获取分配类型ID
    auto const_value = 0;
    
    // 根据变量类型分配不同大小的栈空间
    switch (tp) {
        case IntID: {
            // 整数类型，分配4字节
            const_value = 4;
            func_asm->stack_size += const_value;
        } break;

        case ArrID: {
            // 数组类型，根据数组大小分配空间
            auto mytype = dynamic_cast<ArrType *>(tmp->type.get());
            auto myname = tmp->name;
            auto ptr = mytype->inner;

            // 计算数组总大小：元素数 × 4字节
            const_value = mytype->getSize() * 4;
            func_asm->stack_size += const_value;
        } break;

        case PtrID: {
            // 指针类型，分配4字节
            const_value = 4;
            func_asm->stack_size += const_value;
        } break;

        default: {
            // 未知类型，报错
            printf("Alloca initial type: %d\n", tp);
            assert(false && "unknown initial value type");
        }
    }
    
    // 对于整数类型，直接返回，不需要生成指令
    if (tp == IntID)
        return;

    // 确保栈大小对齐到8字节边界
    if (func_asm->stack_size % 8 != 0) {
        func_asm->stack_size += 8 - (func_asm->stack_size % 8);
    }
    
    // 创建指令计算分配的内存地址：sp - 栈大小
    auto bi_I = new MI_Binary;
    bi_I->op = BINARY_SUBTRACT;
    bi_I->lhs = make_reg(sp);
    bi_I->rhs = make_imm(func_asm->stack_size);
    bi_I->dst = make_operand(alloc->des, mb, true);
    func_asm->local_array_bases.push(bi_I);  // 记录局部数组基址指令
    mb->push((MI *)bi_I);  // 添加指令到基本块
}

// 生成加载指令
// 1: array type, has const offset  
// 2: array type, no const offset   
// 3: reg, simple                   
// 4: reg, global                   
// 5: reg, store in stack    
void emit_Load(Func_Asm *func_asm, InstructionPtr I, Machine_Block *mb, vector<VariablePtr> &globalValues){
    auto Read_I = dynamic_cast<LoadInstruction *>(I.get());
    auto ldr = new MI_Load;  // 创建加载指令

    // 初始化基址寄存器
    ldr->base = make_operand(Read_I->from, mb);

    // 获取被加载的值的实际类型(解引用指针)
    auto type = Read_I->from->type.get();
    while (type->isPtr()) {
        type = dynamic_cast<PtrType *>(type)->inner.get();
    }

    // 处理数组元素加载
    if (my_arr_base.find(Read_I->from) != my_arr_base.end()) {
        if (ptr_val_map.find(Read_I->from) != ptr_val_map.end()) {
            // 情况1: 数组元素，常量索引
            ldr->reg = make_operand(Read_I->to, mb, true);  // 设置目标寄存器
            cur_arr_base = my_arr_base[Read_I->from];  // 获取数组基址
            ldr->base = make_operand(cur_arr_base, mb);  // 设置基址寄存器
            auto opr = ptr_val_map.find(Read_I->from);  // 获取偏移量
            ldr->offset.tag = IMM;
            ldr->offset.value = opr->second * 4;  // 偏移量乘以4(字节)
            cur_arr_base = NULL;
        } else {
            // 情况2: 数组元素，变量索引
            ldr->reg = make_operand(Read_I->to, mb, true);  // 设置目标寄存器
            cur_arr_base = my_arr_base[Read_I->from];  // 获取数组基址
            ldr->base = make_operand(cur_arr_base, mb);  // 设置基址寄存器
            ldr->offset = make_operand(Read_I->from, mb, true);  // 设置偏移量寄存器
            ldr->offset.s_tag = LSL;  // 应用左移操作
            ldr->offset.s_value = 2;  // 左移2位相当于乘以4
            cur_arr_base = NULL;
        }
    } else {
        // 检查是全局变量还是栈变量
        int flag = 0;  // 0=普通变量，1=全局变量，2=栈变量
        for (auto v : globalValues) {
            if (v == Read_I->from) {
                flag = 1;  // 标记为全局变量
                break;
            }
        }
        if (stack_val_map.find(Read_I->from) != stack_val_map.end())
            flag = 2;  // 标记为栈变量
            
        if (flag == 0) {
            // 情况3: 普通变量，直接移动值
            ldr->reg = make_vreg(vreg_count++);  // 创建目标虚拟寄存器
            value_map[Read_I->to] = ldr->reg;  // 更新值映射
            ldr->base = make_operand(Read_I->from, mb);  // 设置源操作数
            emit_move(ldr->reg, ldr->base, mb);  // 生成移动指令
            return;  // 不需要生成加载指令，直接退出
        } else if (flag == 1) {
            // 情况4: 全局变量
            auto nvreg = make_vreg(vreg_count++);  // 创建临时寄存器
            // 加载全局变量地址到临时寄存器
            emit_load_of_global_ref(func_asm, Read_I->from, nvreg, mb);
            ldr->base = nvreg;  // 设置基址寄存器为全局变量地址
            ldr->reg = make_operand(Read_I->to, mb);  // 设置目标寄存器
            value_map[Read_I->to] = ldr->reg;  // 更新值映射
        } else {
            // 情况5: 栈变量
            ldr->reg = make_vreg(vreg_count++);  // 创建目标虚拟寄存器
            value_map[Read_I->to] = ldr->reg;  // 更新值映射
            // 获取栈变量地址
            ldr->base = get_stack_val(Read_I->from, mb, func_asm);
        }
    }
    
    // 检查偏移量是否超出立即数范围(12位)，如果超出则需要先加载到寄存器
    if (ldr->offset.tag == IMM &&
        !can_be_imm12(ldr->offset.value)) {
        auto new_offset = emit_load_of_constant(
            make_vreg(vreg_count++), ldr->offset.value, mb);
        ldr->offset = new_offset->reg;  // 更新偏移量为寄存器操作数
    }
    mb->push((MI *)ldr);  // 将加载指令添加到基本块
}

// 生成存储指令
void emit_Store(Func_Asm *func_asm, InstructionPtr I, Machine_Block *mb, vector<VariablePtr> &globalValues){
    auto Write_I = dynamic_cast<StoreInstruction *>(I.get());
    auto str = new MI_Store;  // 创建存储指令
    
    // 处理数组元素存储
    if (my_arr_base.find(Write_I->des) != my_arr_base.end()) {
        if (ptr_val_map.find(Write_I->des) != ptr_val_map.end()) {
            // 情况1: 数组元素，常量索引
            str->reg = make_operand(Write_I->value, mb, true);  // 设置源寄存器
            cur_arr_base = my_arr_base[Write_I->des];  // 获取数组基址
            str->base = make_operand(cur_arr_base, mb);  // 设置基址寄存器
            auto opr = ptr_val_map.find(Write_I->des);  // 获取偏移量
            str->offset.tag = IMM;
            str->offset.value = opr->second * 4;  // 偏移量乘以4(字节)
            cur_arr_base = NULL;
        } else {
            // 情况2: 数组元素，变量索引
            str->reg = make_operand(Write_I->value, mb, true);  // 设置源寄存器
            cur_arr_base = my_arr_base[Write_I->des];  // 获取数组基址
            str->base = make_operand(cur_arr_base, mb);  // 设置基址寄存器
            str->offset = make_operand(Write_I->des, mb, true);  // 设置偏移量寄存器
            str->offset.s_tag = LSL;  // 应用左移操作
            str->offset.s_value = 2;  // 左移2位相当于乘以4
            cur_arr_base = NULL;
        }
    } else {
        // 检查是全局变量还是栈变量
        int flag = 0;  // 0=局部变量，1=全局变量，2=栈变量
        for (auto v : globalValues) {
            if (v == Write_I->des) {
                flag = 1;  // 标记为全局变量
                break;
            }
        }
        if (stack_val_map.find(Write_I->des) != stack_val_map.end())
            flag = 2;  // 标记为栈变量
            
        // 情况3: 局部变量
        if (flag == 0) {
            str->base = make_operand(Write_I->des, mb, true);  // 设置目标操作数
            str->reg = make_operand(Write_I->value, mb, true);  // 设置源操作数
            emit_move(str->base, str->reg, mb);  // 直接使用移动指令
        }
        // 情况4: 全局变量
        else if (flag == 1) {
            str->reg = make_operand(Write_I->value, mb, true);  // 设置源操作数

            auto nvreg = make_vreg(vreg_count++);  // 创建临时寄存器
            // 加载全局变量地址到临时寄存器
            emit_load_of_global_ref(func_asm, Write_I->des, nvreg, mb);
            str->base = nvreg;  // 设置基址寄存器为全局变量地址
            value_map[Write_I->des] = str->base;  // 更新值映射
        }
        // 情况5: 栈变量
        else {
            // 获取栈变量地址
            str->base = get_stack_val(Write_I->des, mb, func_asm);
            str->reg = make_operand(Write_I->value, mb, true);  // 设置源操作数
        }
    }
    
    // 检查偏移量是否超出立即数范围(12位)，如果超出则需要先加载到寄存器
    if (str->offset.tag == IMM &&
        !can_be_imm12(str->offset.value)) {
        auto new_offset = emit_load_of_constant(
            make_vreg(vreg_count++), str->offset.value, mb);
        str->offset = new_offset->reg;  // 更新偏移量为寄存器操作数
    }
    mb->push((MI *)str);  // 将存储指令添加到基本块
}

// 为函数生成ARM汇编代码
Func_Asm *emit_function_asm(FunctionPtr func, int idx, Module program_module,
                            vector<VariablePtr> &globalValues) {
    // 重置虚拟寄存器计数器
    vreg_count = 0;
    // 清除所有与函数相关的全局映射表
    clear_function_related_variables();

    // 创建新的函数汇编结构
    auto func_asm = new Func_Asm;
    func_asm->index = idx;
    func_asm->name = func->name;
    func_asm->stack_size = 0;

    // 为每个基本块创建对应的机器基本块
    int ii = 0;
    int n = func->basicBlocks.size();
    for (auto bb : func->basicBlocks) {
        func_asm->mbs.push(new Machine_Block());
        func_asm->mbs.back()->i = ii;                     // 设置基本块索引
        func_asm->mbs.back()->label = bb->label;          // 复制标签
        func_asm->mbs.back()->func = func_asm;            // 设置所属函数
        func_asm->mbs.back()->loop_depth = bb->loopDepth; // 复制循环深度
        func_asm->bb2idx[bb] = ii;                        // 建立IR基本块到索引的映射
        func_asm->idx2bb[ii] = bb;                        // 建立索引到IR基本块的映射
        ii++;
    }

    // 建立基本块之间的前驱和后继关系
    for (auto bb : func->basicBlocks) {
        int idx = func_asm->bb2idx[bb];
        // 处理后继基本块
        for (auto succ : bb->getSuccessor()) {
            int succ_idx = func_asm->bb2idx[succ];
            func_asm->mbs[idx]->succs.push(func_asm->mbs[succ_idx]);
        }
        // 处理前驱基本块
        for (auto pred : bb->getPredecessor()) {
            int pred_idx = func_asm->bb2idx[pred];
            func_asm->mbs[idx]->preds.push(func_asm->mbs[pred_idx]);
        }
    }

    // 处理函数参数：前4个参数通过寄存器r0-r3传递
    for (int i = 0; i < 4 && i < func->formArguments.size(); i++) {
        auto arg = func->formArguments[i];
        auto vreg = make_vreg(vreg_count++);          // 为参数创建虚拟寄存器
        value_map[arg] = vreg;                        // 记录参数到虚拟寄存器的映射
        emit_move(vreg, make_reg(i + r0), func_asm->mbs[0]); // 生成从物理寄存器到虚拟寄存器的移动指令
    }

    // 处理超过4个的参数：这些参数通过栈传递
    int cum_offset = 0;
    for (int i = 4; i < func->formArguments.size(); i++) {
        auto arg = func->formArguments[i];
        auto a = make_vreg(vreg_count++);                               // 为参数创建虚拟寄存器
        emit_load_of_later_arg(arg, a, func_asm->mbs[0], cum_offset);   // 生成从栈加载参数的指令
        cum_offset += 4;                                                // 更新栈偏移量，每个参数占4字节
        value_map[arg] = a;                                             // 记录参数到虚拟寄存器的映射
    }

    for (auto bb : func->basicBlocks) {
        Machine_Block *mb = func_asm->mbs[func_asm->bb2idx[bb]];
        for (int i = 0; i < bb->instructions.size(); i++) {
            auto I = bb->instructions[i];
            switch (I->type) {
                case Br: {
                    emit_Branch(func_asm, I, mb);
                } break;
                case Icmp: {
                    emit_Icmp(I, mb);
                } break;
                case Binary: {
                    emit_Binary(I, mb);
                } break;
                case GEP:{
                    emit_Gep(func_asm, I, mb, globalValues);
                } break;
                case Return: {
                    emit_Return(func_asm, I, mb);
                } break;
                case Call: {
                    emit_Call(func_asm, I, mb);
                }break;
                case Alloca: {
                    emit_Alloca(func_asm, I, mb);
                }break;   
                case Load: {
                    emit_Load(func_asm, I, mb, globalValues);   
                }break;
                case Store: {
                    emit_Store(func_asm, I, mb, globalValues);
                }break; 

                case Ext: {
                    // 处理扩展指令(符号扩展或零扩展)
                    auto ext_I = dynamic_cast<ExtInstruction *>(I.get());
                    // 获取源操作数并映射到目标寄存器
                    auto oreg = make_operand(ext_I->from, mb, true);
                    value_map[ext_I->reg] = oreg;  // 直接复用源操作数的寄存器
                } break;
                case Bitcast: {
                    // 处理位类型转换指令，不改变实际位模式，只改变类型解释
                    auto bi_I = dynamic_cast<BitCastInstruction *>(I.get());
                    auto oreg = make_operand(bi_I->from, mb);
                    value_map[bi_I->reg] = oreg;  // 直接复用源操作数的寄存器
                } break;
                case Phi:
                    // Phi指令会在后面单独处理，此处跳过
                    break;
                    
                default: {
                    // 未知指令类型，报错退出
                    assert(false &&
                        "translating unknown IR instructions to assembly");
                } break;
            }
        }
    }

    // 处理Phi指令 - 在SSA形式中用于合并来自不同路径的值
    for (auto bb : func->basicBlocks) {
        auto mb = func_asm->mbs[func_asm->bb2idx[bb]];
        for (int i = 0; i < bb->instructions.size(); i++) {
            // 只处理Phi类型的指令
            if (bb->instructions[i]->type == Phi) {
                // 获取Phi指令
                auto phi =
                    dynamic_cast<PhiInstruction *>(bb->instructions[i].get());
                // 创建一个中间虚拟寄存器，用来接收各个前驱基本块的值
                auto incoming = make_vreg(vreg_count++);

                // 获取Phi指令的结果寄存器
                auto phi_as_operand = make_operand(phi->reg, mb);
                // 在当前基本块的开头添加一条移动指令：phi结果 = incoming
                auto mv = emit_move(phi_as_operand, incoming);
                insert((MI *)mv, mb->inst);  // 插入到基本块的第一条指令之前

                // 为每个Phi的输入源处理前驱基本块
                for (int p = 0; p < phi->from.size(); p++) {
                    auto phi_opr = phi->from[p];  // 获取源值和对应的前驱基本块
                    auto pred_bb = phi_opr.second;  // 前驱基本块
                    int idx = func_asm->bb2idx[pred_bb];  // 获取前驱基本块在函数中的索引
                    auto pred_mb = func_asm->mbs[idx];  // 获取前驱基本块的机器表示

                    // 获取源值的操作数
                    auto from = make_operand(phi_opr.first, pred_mb);
                    // 在前驱基本块末尾添加一条移动指令：incoming = 源值
                    auto mv = emit_move(incoming, from);
                    
                    // 如果前驱块有控制转移指令，在该指令前插入移动指令
                    if (pred_mb->control_transfer_inst) {
                        insert((MI *)mv, pred_mb->control_transfer_inst);
                    } else {
                        // 否则添加到前驱基本块的末尾
                        pred_mb->push((MI *)mv);
                    }
                }
            }
        }
    }
    
    // 更新函数的虚拟寄存器计数
    func_asm->vreg_count = vreg_count;
    return func_asm;  // 返回完成的函数汇编
}

// 生成整个程序的ARM汇编表示
Program_Asm *emit_asm(Module program_module) {
    auto program_asm = new Program_Asm;

    Binary_ir2asm["+"] = BINARY_ADD;
    Binary_ir2asm["-"] = BINARY_SUBTRACT;
    Binary_ir2asm["*"] = BINARY_MULTIPLY;
    Binary_ir2asm["/"] = BINARY_DIVIDE;
    Binary_ir2asm["%"] = BINARY_MOD;
    Binary_ir2asm["!="] = BINARY_NOT_EQUAL_TO;
    Binary_ir2asm["!"] = UNARY_XOR;
    Binary_ir2asm[">"] = BINARY_GREATER_THAN;
    Binary_ir2asm[">="] = BINARY_GREATER_THAN_OR_EQUAL_TO;
    Binary_ir2asm["<"] = BINARY_LESS_THAN;
    Binary_ir2asm["<="] = BINARY_LESS_THAN_OR_EQUAL_TO;
    Binary_ir2asm["=="] = BINARY_EQUAL_TO;
    Binary_ir2asm[","] = BINARY_LSL;
    Binary_ir2asm["."] = BINARY_ASR;

    int i = 0;
    for (auto func : program_module.globalFunctions) {
        if (func->isLib)
            continue;
        i++;
        auto func_asm = emit_function_asm(func, i, program_module,
                                          program_module.globalVariables);
        program_asm->functions.push(func_asm);
    }
    print_program_asm(program_asm, program_module.globalVariables);

    return program_asm;
}

// 生成操作数的汇编文本表示
void build_operand(String_Builder *s, MOperand op) {
    switch (op.tag) {
    // 物理寄存器
    case REG: {
        if (op.value == sp)
            s->append("sp");      // 栈指针
        else if (op.value == lr)
            s->append("lr");      // 链接寄存器
        else if (op.value == pc)
            s->append("pc");      // 程序计数器
        else
            s->append("r%d", op.value); // 普通寄存器
    } break;

    // 虚拟寄存器
    case VREG: {
        s->append("vr%d", op.value);
    } break;

    // 整数立即数
    case IMM: {
        s->append("#%d", op.value);
    } break;

    // 全局变量地址
    case ADR_GLOBAL: {
        s->append("%s", op.adr);
    } break;

    case ERRORTYPE: {
    } break;
    }

    // 处理移位修饰
    switch (op.s_tag) {
    case Nothing:
        break;
    case LSL: {
        s->append(", lsl #%d", op.s_value); // 逻辑左移
    } break;
    case LSR: {
        s->append(", lsr #%d", op.s_value); // 逻辑右移
    } break;
    case ASL: {
        s->append(", asl #%d", op.s_value); // 算术左移
    } break;
    case ASR: {
        s->append(", asr #%d", op.s_value); // 算术右移
    } break;
    }
}

// 生成单个函数的ARM汇编文本
void build_function_asm(String_Builder *s, Func_Asm *func) {
    auto ss = func->mbs.len;
    s->append("%s:\n", func->name.c_str());
    for (int i = 0; i < func->mbs.len; i++) {
        Machine_Block *next_bb = NULL;
        if (i != func->mbs.len - 1)
            next_bb = func->mbs[i + 1];
        s->append(".L%d_%d:\t# %s\n", func->index, func->mbs[i]->i,
                  func->idx2bb[i]->label->name.c_str());
        for (auto I = func->mbs[i]->inst; I; I = I->next) {
            s->append("    ");
            const char *cond = get_branch_suffix(I->cond);
            const char *set_flags = I->update_flags ? "s" : "";
            switch (I->tag) {
            // 生成mov或mvn（mvn为按位取反的mov指令）指令的汇编文本
            case MI_MOVE: {
                auto mv = (MI_Move *)I;
                if (mv->neg) {
                    s->append("mvn%s%s ", set_flags, cond);
                } else {
                    s->append("mov%s%s ", set_flags, cond);
                }
                build_operand(s, mv->dst);
                s->append(", ");
                build_operand(s, mv->src);
            } break;

            // 生成clz（计数前导零）指令的汇编文本
            case MI_CLZ: {
                auto clz = (MI_Clz *)I;
                s->append("clz%s%s ", set_flags, cond);
                build_operand(s, clz->dst);
                s->append(", ");
                build_operand(s, clz->operand);
            } break;

            // 生成二元运算指令的汇编文本
            case MI_BINARY: {
                auto bi = (MI_Binary *)I;

                switch (bi->op) {
                case BINARY_ADD: {
                    // 生成add（加法）指令的汇编文本
                    s->append("add");
                } break;
                case BINARY_SUBTRACT: {
                    // 生成sub（减法）指令的汇编文本
                    s->append("sub");
                } break;
                case BINARY_MULTIPLY: {
                    // 生成mul（乘法）指令的汇编文本
                    s->append("mul");
                } break;
                case BINARY_DIVIDE: {
                    // 生成sdiv（有符号数除法）指令的汇编文本
                    s->append("sdiv");
                } break;
                case BINARY_LSL: {
                    // 生成lsl（逻辑左移）指令的汇编文本
                    s->append("lsl");
                } break;
                case BINARY_LSR: {
                    // 生成lsr（逻辑右移）指令的汇编文本
                    s->append("lsr");
                } break;
                case BINARY_ASL: {
                    // 生成asl（算术左移）指令的汇编文本
                    s->append("asl");
                } break;
                case BINARY_ASR: {
                    // 生成asr（算术右移）指令的汇编文本
                    s->append("asr");
                } break;
                case BINARY_RSB: {
                    // 生成rsb（反向减法）指令的汇编文本
                    s->append("rsb");
                } break;
                case BINARY_BITWISE_AND: {
                    // 生成and（按位与）指令的汇编文本
                    s->append("and");
                } break;
                case BINARY_BITWISE_OR: {
                    // 生成orr（按位或）指令的汇编文本
                    s->append("orr");
                } break;
                case BINARY_BIC: {
                    // 生成bic（按位清零）指令的汇编文本
                    s->append("bic");
                } break;
                case BINARY_SMMUL: {
                    // 生成smmul（高位乘法）指令的汇编文本
                    s->append("smmul");
                } break;
                default: {
                    exit(53);
                    assert(false && "unknown binary asm instruction.");
                }
                }
                s->append("%s%s", set_flags, cond);
                s->append(" ");
                build_operand(s, bi->dst);
                s->append(", ");
                build_operand(s, bi->lhs);
                s->append(", ");
                build_operand(s, bi->rhs);
            } break;

            // 生成乘加融合指令的汇编文本
            case MI_COMPLEXMUL: {
                auto complex = (MI_ComplexMul *)I;
                switch (complex->op) {
                // 生成mla（乘加）指令的汇编文本
                case COMPLEX_MLA: {
                    s->append("mla");
                } break;

                // 生成mls（乘减）指令的汇编文本
                case COMPLEX_MLS: {
                    s->append("mls");
                } break;
                }
                s->append("%s%s", set_flags, cond);
                s->append(" ");
                build_operand(s, complex->dst);
                s->append(", ");
                build_operand(s, complex->lhs);
                s->append(", ");
                build_operand(s, complex->rhs);
                s->append(", ");
                build_operand(s, complex->extra);
            } break;
        
            // 生成比较指令的汇编文本
            case MI_COMPARE: {
                auto cmp = (MI_Compare *)I;

                if (cmp->neg) {
                    s->append("cmn ");
                } else {
                    s->append("cmp ");
                }
                build_operand(s, cmp->lhs);
                s->append(", ");
                build_operand(s, cmp->rhs);
            } break;

            // 生成条件分支指令的汇编文本
            case MI_BRANCH: {
                auto br = (MI_Branch *)I;
                // 如果下一个基本块已被条件化，跳过分支指令
                if (next_bb && next_bb->condified)
                    break;
                // 无条件跳转，且目标不是下一个基本块
                if (br->cond == NO_CONDITION) {
                    if (br->true_target->i != i + 1) {
                        s->append("b .L%d_%d", func->index, br->true_target->i);
                    }
                }
                // 条件跳转，true分支为下一个基本块，生成反条件跳转到false分支
                else if (br->true_target->i == i + 1) {
                    s->append("b%s .L%d_%d\n",
                              get_branch_suffix(invert_branch_cond(br->cond)),
                              func->index, br->false_target->i);
                } 
                // 条件跳转，false分支为下一个基本块，生成条件跳转到true分支
                else if (br->false_target->i == i + 1) {
                    s->append("b%s .L%d_%d\n", get_branch_suffix(br->cond),
                              func->index, br->true_target->i);
                } 
                // 其他情况，先跳转到true分支，再无条件跳转到false分支
                else {
                    s->append("b%s .L%d_%d\n", get_branch_suffix(br->cond),
                              func->index, br->true_target->i);
                    s->append("    ");
                    s->append("b .L%d_%d", func->index, br->false_target->i);
                }
            } break;

            // 生成push或pop指令的汇编文本
            case MI_PUSH:
            case MI_POP: {
                auto push_or_pop = (MI_Push *)I;

                s->append((I->tag == MI_PUSH) ? "push {" : "pop {");
                for (int i = 0; i < push_or_pop->operands.len; i++) {
                    build_operand(s, push_or_pop->operands[i]);
                    if (i != push_or_pop->operands.len - 1)
                        s->append(", ");
                }
                s->append("}");
            } break;

            // 生成函数调用指令的汇编文本
            case MI_FUNC_CALL: {
                auto call = (MI_Func_Call *)I;
                s->append("bl%s %s", cond, call->func_name);
            } break;

            // 生成load/store指令的汇编文本
            case MI_LOAD:
            case MI_STORE: {
                auto load_or_store = (MI_Load *)I;
                // 基址为全局变量，使用movw/movt加载地址
                if (load_or_store->base.tag == ADR_GLOBAL) {
                    s->append("movw%s ", cond);
                    build_operand(s, load_or_store->reg);
                    s->append(", ");
                    s->append(":lower16:");
                    build_operand(s, load_or_store->base);
                    s->append("\n    movt%s ", cond);
                    build_operand(s, load_or_store->reg);
                    s->append(", ");
                    s->append(":upper16:");
                    build_operand(s, load_or_store->base);
                } 
                // 基址为立即数，根据不同情况选择mov/movw/mvn/movt指令
                else if (load_or_store->base.tag == IMM) {
                    auto val = load_or_store->base.value;
                    // 可以用mov指令直接加载
                    if (can_be_imm_ror(val)) {
                        s->append("mov%s ", cond);
                        build_operand(s, load_or_store->reg);
                        s->append(", ");
                        build_operand(s, load_or_store->base);
                    }
                    // 16位无符号立即数，使用movw
                    else if ((val & 0xFFFF) == val) {
                        s->append("movw%s ", cond);
                        build_operand(s, load_or_store->reg);
                        s->append(", ");
                        build_operand(s, load_or_store->base);
                    }
                    // -1 ~ -257 范围，使用mvn取反加载
                    else if (val < 0 && val > -258) {
                        s->append("mvn%s ", cond);
                        build_operand(s, load_or_store->reg);
                        s->append(", ");
                        auto valn = -val - 1;
                        MOperand valn_imm(IMM, valn);
                        build_operand(s, valn_imm);
                    } 
                    // 其他情况，拆分为movw/movt加载高低16位
                    else {
                        auto vall = val & 0xFFFF;
                        auto valh = (val >> 16) & 0xFFFF;
                        MOperand vall_imm(IMM, vall);
                        MOperand valh_imm(IMM, valh);
                        s->append("movw%s ", cond);
                        build_operand(s, load_or_store->reg);
                        s->append(", ");
                        build_operand(s, vall_imm);
                        s->append("\n    movt%s ", cond);
                        build_operand(s, load_or_store->reg);
                        s->append(", ");
                        build_operand(s, valh_imm);
                    }
                }
                // 其他情况，生成标准的ldr/str指令
                else {
                    s->append(I->tag == MI_LOAD ? "ldr" : "str");
                    s->append(cond);
                    s->append(" ");
                    build_operand(s, load_or_store->reg);
                    s->append(", [");
                    build_operand(s, load_or_store->base);
                    if (load_or_store->offset.tag != ERRORTYPE) {
                        s->append(", ");
                        build_operand(s, load_or_store->offset);
                    }
                    s->append("]");
                }
            } break;

            // 生成返回指令的汇编文本
            case MI_RETURN: {
                s->append("bx lr");
            } break;
            }
            s->append("\n");
        }
    }
}

// 生成整个程序的ARM汇编文本
void build_program_asm(String_Builder *s, Program_Asm *pro,
                       vector<VariablePtr> &globalVariables) {
    s->append(".arch armv8-a\n");

    build_globals(s, globalVariables);
    s->append("\n");

    s->append(".text\n");
    s->append(".align 2\n");
    s->append(".syntax unified\n");
    s->append(".arm\n");
    s->append(".fpu neon\n");

    s->append(".global main\n\n");
    for (int i = 0; i < pro->functions.len; i++) {
        build_function_asm(s, pro->functions[i]);
        s->append("\n\n");
    }
}

// 打印操作数的汇编文本表示
void print_operand(MOperand op) {
    String_Builder s;
    build_operand(&s, op);
    // printf("%s", s.c_str());
}

// 打印单个函数的ARM汇编文本
void print_function_asm(Func_Asm *func) {
    String_Builder s;
    build_function_asm(&s, func);
    // printf("%s", s.c_str());
}

// 打印整个程序的ARM汇编文本
void print_program_asm(Program_Asm *pro, vector<VariablePtr> &globalVariables) {
    String_Builder s;
    build_program_asm(&s, pro, globalVariables);
    // printf("%s", s.c_str());
}