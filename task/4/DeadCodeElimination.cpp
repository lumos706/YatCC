#include "DeadCodeElimination.hpp"
#include <unordered_set>

using namespace llvm;

PreservedAnalyses
DeadCodeElimination::run(Module& mod, ModuleAnalysisManager& mam)
{
    int eliminated = 0;

    // 1. 标记所有被load过的全局变量
    std::unordered_set<GlobalVariable*> usedGlobals;
    for (auto& func : mod) {
        for (auto& bb : func) {
            for (auto& inst : bb) {
                if (auto* loadInst = dyn_cast<LoadInst>(&inst)) {
                    if (auto* gv = dyn_cast<GlobalVariable>(loadInst->getPointerOperand())) {
                        usedGlobals.insert(gv);
                    }
                }
            }
        }
    }

    // 2. 删除未被load过的全局变量的store指令
    for (auto& func : mod) {
        for (auto& bb : func) {
            std::vector<Instruction*> toErase;
            for (auto& inst : bb) {
                if (auto* storeInst = dyn_cast<StoreInst>(&inst)) {
                    if (auto* gv = dyn_cast<GlobalVariable>(storeInst->getPointerOperand())) {
                        if (!usedGlobals.count(gv)) {
                            toErase.push_back(storeInst);
                            ++eliminated;
                        }
                    }
                }
            }
            for (auto* inst : toErase)
                inst->eraseFromParent();
        }
    }

    // 3. 普通DCE：删除无用指令（无副作用且无use）
    bool changed = true;
    while (changed) {
        changed = false;
        for (auto& func : mod) {
            for (auto& bb : func) {
                std::vector<Instruction*> toErase;
                for (auto& inst : bb) {
                    // 跳过有副作用的指令
                    if (inst.isTerminator() || inst.mayHaveSideEffects() || inst.isEHPad())
                        continue;
                    // 如果没有use，可以删除
                    if (inst.use_empty()) {
                        toErase.push_back(&inst);
                        ++eliminated;
                        changed = true;
                    }
                }
                for (auto* inst : toErase)
                    inst->eraseFromParent();
            }
        }
    }

    mOut << "DeadCodeElimination running...\nTo eliminate " << eliminated << " instructions\n";
    return PreservedAnalyses::all();
}