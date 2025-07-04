#include "ConstantPropagation.hpp"
#include <unordered_set>

using namespace llvm;

static bool isInStoreList(Value* v, const std::unordered_set<Value*>& storeSet) {
    return storeSet.find(v) != storeSet.end();
}

static int tryReplaceOperands(Instruction& inst, std::unordered_map<Value*, ConstantInt*>& intList) {
    int changed = 0;
    for (unsigned i = 0; i < inst.getNumOperands(); ++i) {
        Value* operand = inst.getOperand(i);
        auto it = intList.find(operand);
        if (it != intList.end() && operand->getType()->isIntegerTy()) {
            inst.setOperand(i, ConstantInt::getSigned(operand->getType(), it->second->getSExtValue()));
            ++changed;
        }
    }
    return changed;
}

PreservedAnalyses
ConstantPropagation::run(Module& mod, ModuleAnalysisManager& mam)
{
    int constPropagationTimes = 0;
    std::unordered_set<Value*> storeSet;
    std::unordered_map<Value*, ConstantInt*> intList;

    // 收集所有被store修改过的变量
    for (auto& func : mod)
        for (auto& bb : func)
            for (auto& inst : bb)
                if (auto storeInst = dyn_cast<StoreInst>(&inst))
                    storeSet.insert(storeInst->getPointerOperand());

    // 收集未被store过且有初值的全局变量
    for (auto& global : mod.globals()) {
        if (global.hasInitializer()) {
            if (auto constVal = dyn_cast<ConstantInt>(global.getInitializer())) {
                if (!isInStoreList(&global, storeSet))
                    intList[&global] = constVal;
            }
        }
    }

    // 遍历所有指令，替换load和常量传播
    for (auto& func : mod) {
        for (auto& bb : func) {
            std::vector<Instruction*> instToErase;
            for (auto& inst : bb) {
                // 替换load指令
                if (auto loadInst = dyn_cast<LoadInst>(&inst)) {
                    auto pointer = loadInst->getPointerOperand();
                    auto it = intList.find(pointer);
                    if (it != intList.end()) {
                        loadInst->replaceAllUsesWith(ConstantInt::getSigned(loadInst->getType(), it->second->getSExtValue()));
                        instToErase.push_back(loadInst);
                        ++constPropagationTimes;
                    }
                }
                // 替换二元运算和整数比较的操作数
                if (isa<BinaryOperator>(inst) || isa<ICmpInst>(inst)) {
                    constPropagationTimes += tryReplaceOperands(inst, intList);
                }
            }
            for (auto* i : instToErase)
                i->eraseFromParent();
        }
    }

    mOut << "ConstantPropagation running...\nTo eliminate " << constPropagationTimes
         << " instructions\n";
    return PreservedAnalyses::all();
}