
#include "asm_passes.h"
#include "../arm.h"
#include <iostream>

// Optimization passes
#include "stack_ra.h"
#include "register_allocation.h"
#include "simplify_asm.h"
#include "remove_identical_moves.h"
#include "block_placement.h"
#include "remove_redundant_ldrs.h"
#include "condify.h"
#include "use_analysis.h"
#include "algebraic_identity.h"

inline void run_on_every_function(Program_Asm *program_asm, Asm_Func_Pass pass) {
    for(auto func_asm : program_asm->functions) {
        pass(func_asm);
    }
}

#define RUN(pass_name) pass_name(program_asm)
void bless(Program_Asm *program_asm, bool opt) {
	RUN(remove_redundant_ldrs);
    RUN(use_analysis);
    RUN(algebraic_identity);
    register_allocation(program_asm, opt);
    RUN(remove_identical_moves);
    RUN(remove_redundant_ldrs);
    // RUN(stack_ra);
    if (opt) {
        // RUN(simplify_asm);
        // RUN(remove_redundant_ldrs);
        // RUN(block_placement);
        // RUN(condify);
    }
}
#undef RUN

// helper functions

void replace_uses(MI *I, MOperand *old_opr, MOperand *new_opr, Func_Asm *func) {
    Array<MOperand *> oprs = get_uses_ptr(I, func->has_return_value);
    printf("replace use: ");
    print_operand(*old_opr);
    printf(" <- ");
    print_operand(*new_opr);
    printf("\n");
    for(MOperand *opr_ptr : oprs) {
        if (opr_ptr && *opr_ptr == *old_opr) {
            print_operand(*opr_ptr);
            printf(" found: old_opr.s_tag %d, old_opr.s_value %d, ", old_opr->s_tag, old_opr->s_value);
            // modify use
            auto use = opr_ptr->get_use_head(func);
            while(use != nullptr) {
                if(use->I == I) {
                    use->rm_use(func);
                    auto new_use = new MI_Use(new_opr, use->user, I);
                    // user is nullptr when MI_Store
                    if(use->user != nullptr)
                        use->user->add_use(new_use, func);
                }
                use = use->next;
            }
            // keep 'LSL' attribute
            opr_ptr->tag = new_opr->tag;
            opr_ptr->value = new_opr->value;
            opr_ptr->fvalue = new_opr->fvalue;
            opr_ptr->adr = new_opr->adr;
            printf("opr_ptr.s_tag %d, opr_ptr.s_value %d\n", opr_ptr->s_tag, opr_ptr->s_value);
        }
    }
}

void replace_defs(MI *I, MOperand *old_opr, MOperand *new_opr, Func_Asm *func) {
    Array<MOperand *> oprs = get_defs_ptr(I);
    for(MOperand *opr_ptr : oprs) {
        if (opr_ptr && *opr_ptr == *old_opr) {
            // modify use
            auto uses = get_uses_ptr(I, func->has_return_value);
            for(auto use_operand: uses) {
                rm_instruction_use(I, use_operand, func);
                auto new_use = new MI_Use(use_operand, new_opr, I);
                use_operand->add_use(new_use, func);
            }
            // keep 'LSL' attribute
            opr_ptr->tag = new_opr->tag;
            opr_ptr->value = new_opr->value;
            opr_ptr->fvalue = new_opr->fvalue;
            opr_ptr->adr = new_opr->adr;
        }
    }
}

Array<MOperand> get_defs(MI *I) {
    Array<MOperand> oprs;
    switch(I->tag) {
        case MI_MOVE:   { oprs.push(((MI_Move *)I)->dst); } break;
        case MI_VMOVE: {
            if(((MI_VMove *)I)->dst.tag == REG || ((MI_VMove *)I)->dst.tag == VREG ){
                oprs.push(((MI_VMove *)I)->dst);
            }
        } break;
        case MI_BINARY: { oprs.push(((MI_Binary *)I)->dst); } break;
        case MI_COMPLEXMUL: { oprs.push(((MI_ComplexMul *)I)->dst); } break;
        case MI_FUNC_CALL: {
            oprs.push(make_reg(lr));
            for(uint8 r = r0; r < REG_COUNT; r++) {
                if (is_caller_save(r)) {
                    oprs.push(make_reg(r));
                }
            }
        } break;

        case MI_LOAD: { oprs.push(((MI_Load *)I)->reg); } break;

        case MI_CLZ: { oprs.push(((MI_Clz *)I)->dst); } break;

        case MI_VCVT: {} break;

        /*
        case MI_POP:  {
            auto pop = (MI_Pop *) I;
            for (auto pop_operand : pop->operands) {
                oprs.push(pop_operand);
            }
        } break;
        */

        case MI_COMPARE:
        case MI_PUSH:
        case MI_RETURN:
        case MI_POP:
        case MI_STORE: // @TODO: store/load may use self increment thing?
        case MI_BRANCH: 
        //----float----
        case MI_VBINARY:
        case MI_VCOMPARE:
        case MI_VPUSH:
        case MI_VPOP:
        case MI_VLOAD: // vldr
        case MI_VSTORE:  // vstr
            break;
        
        default: {
            fprintf(stderr, "unknown instrID: %d\n", I->tag);
            assert(false && "meet unknown instr in get_defs");
        }
    }
    return oprs;
}

Array<MOperand> get_uses(MI *I, bool func_has_return_value) {
    Array<MOperand> oprs;

    switch(I->tag) {

        case MI_MOVE:   {
            if(((MI_Move *)I)->src.tag == REG || ((MI_Move *)I)->src.tag == VREG){
                oprs.push(((MI_Move *)I)->src);
            }
        } break;

        case MI_VMOVE: {
            if(((MI_VMove *)I)->src.tag == REG || ((MI_VMove *)I)->src.tag == VREG ){
                oprs.push(((MI_VMove *)I)->src);
            }
        } break;
        case MI_CLZ:   {
            oprs.push(((MI_Clz *)I)->operand);
        } break;

        case MI_BINARY: {
            oprs.push(((MI_Binary *)I)->lhs);
            oprs.push(((MI_Binary *)I)->rhs);
        } break;

        case MI_COMPLEXMUL: {
            oprs.push(((MI_ComplexMul *)I)->lhs);
            oprs.push(((MI_ComplexMul *)I)->rhs);
            oprs.push(((MI_ComplexMul *)I)->extra);
        } break;

        case MI_COMPARE: {
            oprs.push(((MI_Compare *)I)->lhs);
            oprs.push(((MI_Compare *)I)->rhs);
        } break;

        case MI_FUNC_CALL: {
            auto call = (MI_Func_Call *)I;
            // @TODO: get uses based on arg count
            for(uint8 r = r0; r <= r3; r++) {
                if (r >= call->arg_count + r0) break;
                oprs.push(make_reg(r));
            }
        } break;

        case MI_STORE: { 
            auto store = (MI_Store *)I;
            oprs.push(store->reg);
            if(store->base.tag != ERRORTYPE)
                oprs.push(store->base);
            if(store->offset.tag != ERRORTYPE)
                oprs.push(store->offset);
        } break;

        case MI_LOAD: { 
            auto load = (MI_Load *)I;
            if(load->base.tag != ERRORTYPE)
                oprs.push(load->base);
            if(load->offset.tag != ERRORTYPE)
                oprs.push(load->offset);
        } break;

        case MI_PUSH: {
            auto push = (MI_Push *) I;
            for (auto push_operand : push->operands) {
                oprs.push(push_operand);
            }
        } break;

        case MI_RETURN: {
            oprs.push(make_reg(lr));
            if (func_has_return_value) {
                oprs.push(make_reg(r0));
            }
        } break;

        case MI_VLOAD:{
            auto vldr = (MI_VLoad *)I;
            if(vldr->base.tag != ERRORTYPE){
                oprs.push(vldr->base);
            }
            if(vldr->offset.tag != ERRORTYPE){
                oprs.push(vldr->offset);
            }
        } break;
        case MI_VSTORE:{
            auto vstr = (MI_VStore *)I;
            if(vstr->base.tag != ERRORTYPE){
                oprs.push(vstr->base);
            }
            if(vstr->offset.tag != ERRORTYPE){
                oprs.push(vstr->offset);
            }
        } break;
        case MI_VCVT: {} break;

        case MI_POP:
        case MI_BRANCH: 
        //----float----
        case MI_VBINARY:
        case MI_VCOMPARE:
        case MI_VPUSH:
        case MI_VPOP:
            break;
        default: {
            fprintf(stderr, "unknown instrID: %d\n", I->tag);
            assert(false && "meet unknown instr in get_uses");
        }
    }

    return oprs;
}

/******************************************************************************
 * get pointer of uses or defs(modify uses)
 ******************************************************************************
*/

Array<MOperand *> get_defs_ptr(MI *I) {
    Array<MOperand *> oprs;
    switch(I->tag) {
        case MI_MOVE:   { oprs.push(&((MI_Move *)I)->dst); } break;
        case MI_VMOVE: {
            if(((MI_VMove *)I)->dst.tag == REG || ((MI_VMove *)I)->dst.tag == VREG) {
                oprs.push(&((MI_VMove *)I)->dst);
            }
        } break;
        case MI_BINARY: { oprs.push(&((MI_Binary *)I)->dst); } break;
        case MI_COMPLEXMUL: { oprs.push(&((MI_ComplexMul *)I)->dst); } break;
        case MI_FUNC_CALL: {} break;

        case MI_LOAD: { oprs.push(&((MI_Load *)I)->reg); } break;

        // use 'nullptr' as a placeholder
        case MI_STORE: {
            oprs.push(nullptr);
        } break;

        case MI_CLZ: { oprs.push(&((MI_Clz *)I)->dst); } break;

        case MI_VCVT: {} break;

        /*
        case MI_POP:  {
            auto pop = (MI_Pop *) I;
            for (auto pop_operand : pop->operands) {
                oprs.push(pop_operand);
            }
        } break;
        */

        case MI_COMPARE:
        case MI_PUSH:
        case MI_RETURN:
        case MI_POP:
        case MI_BRANCH: 
        //----float----
        case MI_VBINARY:
        case MI_VCOMPARE:
        case MI_VPUSH:
        case MI_VPOP:
        case MI_VLOAD:
        case MI_VSTORE:
            break;
        
        default: {
            fprintf(stderr, "unknown instrID: %d\n", I->tag);
            assert(false && "meet unknown instr in get_defs");
        }
    }
    return oprs;
}

Array<MOperand *> get_uses_ptr(MI *I, bool func_has_return_value) {
    Array<MOperand *> oprs;

    switch(I->tag) {

        case MI_MOVE:   {
            if(((MI_Move *)I)->src.tag == REG || ((MI_Move *)I)->src.tag == VREG){
                oprs.push(&((MI_Move *)I)->src);
            }
        } break;

        case MI_VMOVE: {
            if(((MI_VMove *)I)->src.tag == REG || ((MI_VMove *)I)->src.tag == VREG ){
                oprs.push(&((MI_VMove *)I)->src);
            }
        } break;
        case MI_CLZ:   {
            oprs.push(&((MI_Clz *)I)->operand);
        } break;

        case MI_BINARY: {
            oprs.push(&((MI_Binary *)I)->lhs);
            oprs.push(&((MI_Binary *)I)->rhs);
        } break;

        case MI_COMPLEXMUL: {
            oprs.push(&((MI_ComplexMul *)I)->lhs);
            oprs.push(&((MI_ComplexMul *)I)->rhs);
            oprs.push(&((MI_ComplexMul *)I)->extra);
        } break;

        case MI_COMPARE: {
            oprs.push(&((MI_Compare *)I)->lhs);
            oprs.push(&((MI_Compare *)I)->rhs);
        } break;

        case MI_FUNC_CALL: {} break;

        case MI_STORE: { 
            auto store = (MI_Store *)I;
            oprs.push(&store->reg);
            if(store->base.tag != ERRORTYPE)
                oprs.push(&store->base);
            if(store->offset.tag != ERRORTYPE)
                oprs.push(&store->offset);
        } break;

        case MI_LOAD: { 
            auto load = (MI_Load *)I;
            if(load->base.tag != ERRORTYPE)
                oprs.push(&load->base);
            if(load->offset.tag != ERRORTYPE)
                oprs.push(&load->offset);
        } break;

        case MI_PUSH: {} break;

        case MI_RETURN: {} break;

        case MI_VLOAD:{
            auto vldr = (MI_VLoad *)I;
            if(vldr->base.tag != ERRORTYPE){
                oprs.push(&vldr->base);
            }
            if(vldr->offset.tag != ERRORTYPE){
                oprs.push(&vldr->offset);
            }
        } break;
        case MI_VSTORE:{
            auto vstr = (MI_VStore *)I;
            if(vstr->base.tag != ERRORTYPE){
                oprs.push(&vstr->base);
            }
            if(vstr->offset.tag != ERRORTYPE){
                oprs.push(&vstr->offset);
            }
        } break;
        case MI_VCVT: {} break;

        case MI_POP:
        case MI_BRANCH: 
        //----float----
        case MI_VBINARY:
        case MI_VCOMPARE:
        case MI_VPUSH:
        case MI_VPOP:
            break;
        default: {
            fprintf(stderr, "unknown instrID: %d\n", I->tag);
            assert(false && "meet unknown instr in get_uses");
        }
    }

    return oprs;
}

// delete uses that 'opr' uses
void delete_user(MOperand *opr, Func_Asm *func) {
    auto I = opr->get_def_I(func);
    if(I != nullptr) {
        auto use_operands = get_uses(I, func->has_return_value);
        for(auto use_operand: use_operands) {
            auto use = use_operand.get_use_head(func);
            while(use != nullptr) {
                if(use->I == I) {
                    use->rm_use(func);
                }
                use = use->next;
            }
        }
    }
}

// replace all uses of 'opr' by 'new_opr'
void replace_operand_by_operand(MOperand *opr, MOperand *new_opr, Func_Asm *func) {
    auto use = opr->get_use_head(func);
    while(use != nullptr) {
        auto user = use->user;
        assert(use->I != nullptr);
        replace_uses(use->I, opr, new_opr, func);
        use = use->next;
    }
    delete_user(opr, func);
}
