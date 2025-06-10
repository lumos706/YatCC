#include "asm_passes.h"
#include "../arm.h"

void use_analysis(Program_Asm *prog) {
    for(auto f : prog->functions) {
        f->use_head_map.clear();
        f->def_I_map.clear();
        for(auto mb : f->mbs) {
            for(auto I = mb->inst; I; I=I->next) {
                auto defs = get_defs_ptr(I);
                auto uses = get_uses_ptr(I, f->has_return_value);
                for(auto def: defs) {
                    for(auto use: uses) {
                        auto mi_use = new MI_Use(use, def, I);
                        use->add_use(mi_use, f);
                    }
                    if(def != nullptr)
                        def->set_def_I(I, f);
                }
            }
        }
    }
}