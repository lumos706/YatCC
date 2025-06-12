#include "asm_passes.h"

void remove_redundant_ldrs(Program_Asm *prog) {
	for (auto f : prog->functions) {
		for (auto mb : f->mbs) {
			for (auto I = mb->inst; I; I = I->next) 
			{
				if (I->tag == MI_STORE && I->next)
				{
					if (I->next->tag == MI_LOAD)
					{ 
						auto str = (MI_Store *)I;
						auto ldr = (MI_Load *)I->next;
                        if (str->base == ldr->base) {
                            if (str->offset.tag == ERRORTYPE || (str->offset == ldr->offset))
                            {
                                if (!(str->reg == ldr->reg)) {
                                    auto mv = new MI_Move(ldr->reg, str->reg);
                                    insert(mv, ldr);
                                    I = mv;
                                }
                                ldr->mark();
                            }
                        }
					}
				}
				else if (I->tag == MI_VSTORE && I->next)
				{
					if (I->next->tag == MI_VLOAD)
					{ 
						auto str = (MI_VStore *)I;
						auto ldr = (MI_VLoad *)I->next;
                        if (str->base == ldr->base) {
                            if (str->offset.tag == ERRORTYPE || (str->offset == ldr->offset))
                            {
                                if (!(str->reg == ldr->reg)) {
                                    auto mv = new MI_VMove(ldr->reg, str->reg, true);
                                    insert(mv, ldr);
                                    I = mv;
                                }
                                ldr->mark();
                            }
                        }
					}
				}
			}
            mb->erase_marked_values();
		}
	}
}
