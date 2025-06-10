#include "asm_passes.h"
#include "../arm.h"

bool is_exp(int n) {
    if (n & 1) return false;
    int bitCount = 0;
    for(int i = 0; i < 31; i ++) 
    {
        n >>= 1;
        bitCount += (n & 1);
    }
    return (bitCount == 1);
}

void algebraic_identity(Program_Asm *prog) {
    // all operands are SSA
    int erase_count = 0;
    auto zero_reg = make_reg(0);
    for(auto f : prog->functions) {
        for(auto mb : f->mbs) {
            for(auto I = mb->inst; I; I=I->next) {
                if (I->tag == MI_BINARY) {
                    auto binary = (MI_Binary *) I;
                    switch (binary->op) {
                        case BINARY_ADD: {
                            // 0 + x
                            if (binary->lhs.tag == IMM && binary->lhs.value == 0 || binary->lhs == zero_reg) {
                                replace_operand_by_operand(&binary->dst, &binary->rhs, f);
                                I->mark();
                            }
                                
                            // x + 0
                            else if (binary->rhs.tag == IMM && binary->rhs.value == 0 || binary->rhs == zero_reg) {
                                replace_operand_by_operand(&binary->dst, &binary->lhs, f);
                                I->mark();
                            }

                            // x + x
                            else if (binary->lhs == binary->rhs && binary->rhs.s_tag == Nothing) {
                                auto lsl = new MI_Binary(BINARY_LSL, binary->dst, binary->lhs, make_imm(1));
                                insert(lsl, I);
                                I->mark();
                            }
                        } break;
                        case BINARY_SUBTRACT: {
                            // x - 0
                            if (binary->rhs.tag == IMM && binary->rhs.value == 0 || binary->rhs == zero_reg) {
                                replace_operand_by_operand(&binary->dst, &binary->lhs, f);
                                I->mark();
                            }
                                
                            // x - x
                            else if (binary->lhs == binary->rhs && binary->rhs.s_tag == Nothing) {
                                replace_operand_by_operand(&binary->dst, &zero_reg, f);
                                I->mark();
                            }
                        } break;
                        case BINARY_MULTIPLY: {
                            // 0 * x
                            if (binary->lhs.tag == IMM && binary->lhs.value == 0) {
                                replace_operand_by_operand(&binary->dst, &zero_reg, f);
                                I->mark();
                            }
                                
                            // x * 0
                            else if (binary->rhs.tag == IMM && binary->rhs.value == 0) {
                                replace_operand_by_operand(&binary->dst, &zero_reg, f);
                                I->mark();
                            }

                            // x * 2^sh
                            else if (binary->rhs.tag == IMM && is_exp(binary->rhs.value)) {
                                auto lsl = new MI_Binary(BINARY_LSL, binary->dst, binary->lhs, make_imm(int(log2(binary->rhs.value))));
                                insert(lsl, I);
                                I->mark();
                            }

                            // x * (2^sh - 1)
                            else if (binary->rhs.tag == IMM && is_exp(binary->rhs.value + 1)) {
                                auto shmat = int(log2(binary->rhs.value + 1));
                                auto lsl_lhs = binary->lhs;
                                lsl_lhs.s_tag = LSL;
                                lsl_lhs.s_value = shmat;
                                auto lsl = new MI_Binary(BINARY_RSB, binary->dst, binary->lhs, lsl_lhs);
                                insert(lsl, I);
                                I->mark();
                            }
                        } break;
                        default: break;
                    }
                    if(I->marked) {
                        auto binary = (MI_Binary *)(I);
                        printf("erase instruction: ");
                        print_operand(binary->dst);
                        printf(",");
                        print_operand(binary->lhs);
                        printf(",");
                        print_operand(binary->rhs);
                        printf("   op_type: %d\n", binary->op);
                        erase_count ++;
                    }
                }
            }
            mb->erase_marked_values();
        }
    }
}