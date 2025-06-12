#ifndef ASM_PASSES_H
#define ASM_PASSES_H

// @Lame
// This is copied from IR passes

#include "../general.h"
#include "../arm.h"

#define ASM_FUNC_PASS(pass_name) void pass_name(Func_Asm *func_asm)
typedef ASM_FUNC_PASS(Asm_Func_Pass);

#define ASM_PASS(pass_name) void pass_name(Program_Asm *program_asm)
typedef ASM_PASS(Asm_Pass);

inline void run_on_every_function(Program_Asm *program_asm, Asm_Func_Pass pass);

void bless(Program_Asm *program_asm, bool opt = false);

void replace_uses(MI *I, MOperand *old_opr, MOperand *new_opr, Func_Asm *func);
void replace_defs(MI *I, MOperand *old_opr, MOperand *new_opr, Func_Asm *func);
Array<MOperand> get_defs(MI *I);
Array<MOperand> get_uses(MI *I, bool func_has_return_value);
Array<MOperand *> get_defs_ptr(MI *I);
Array<MOperand *> get_uses_ptr(MI *I, bool func_has_return_value);

void replace_operand_by_operand(MOperand *opr, MOperand *new_opr, Func_Asm *func);
void delete_user(MOperand *opr, Func_Asm *func);

#endif
