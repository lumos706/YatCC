#include "Inline.hpp"
#include <llvm/IR/IRBuilder.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include <set>
#include <queue>

using namespace llvm;

// 判断函数是否递归（DFS检测环）
static bool isRecursive(Function* F, std::set<Function*>& stack) {
    if (stack.count(F)) return true;
    stack.insert(F);
    for (auto& BB : *F) {
        for (auto& I : BB) {
            if (auto* call = dyn_cast<CallInst>(&I)) {
                Function* callee = call->getCalledFunction();
                if (callee && !callee->isDeclaration()) {
                    if (isRecursive(callee, stack)) return true;
                }
            }
        }
    }
    stack.erase(F);
    return false;
}

PreservedAnalyses 
Inline::run(Module& mod, ModuleAnalysisManager& mam)
{
    int inlineTimes = 0;
    std::vector<CallInst*> callsToInline;

    // 1. 收集所有可内联的调用点（非递归、非外部、非main）
    for (auto& F : mod) {
        if (F.isDeclaration() || F.getName() == "main") continue;
        std::set<Function*> stack;
        if (isRecursive(&F, stack)) continue;
        for (auto& BB : F) {
            for (auto it = BB.begin(); it != BB.end(); ++it) {
                if (auto* call = dyn_cast<CallInst>(&*it)) {
                    Function* callee = call->getCalledFunction();
                    if (callee && !callee->isDeclaration() && callee->getName() != "main") {
                        std::set<Function*> calleeStack;
                        if (!isRecursive(callee, calleeStack)) {
                            callsToInline.push_back(call);
                        }
                    }
                }
            }
        }
    }

    // 2. 执行内联
    for (auto* call : callsToInline) {
        Function* callee = call->getCalledFunction();
        if (!callee || callee->isDeclaration()) continue;

        InlineFunctionInfo IFI;
        auto IR = InlineFunction(*call, IFI);
        if (IR.isSuccess()) {
            ++inlineTimes;
        }
    }

    mOut << "Inline running...\nTo inline " << inlineTimes << " calls\n";
    return PreservedAnalyses::all();
}