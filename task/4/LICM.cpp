#include "LICM.hpp"
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/CFG.h>
#include <set>
#include <vector>
#include <stack>
#include <unordered_map>

using namespace llvm;

// 判断一个指令是否循环无关（所有操作数都在循环外部定义，且无副作用）
static bool isLoopInvariant(Instruction* inst, const std::set<BasicBlock*>& loopBlocks) {
    if (inst->mayHaveSideEffects() || inst->isTerminator() || inst->isEHPad())
        return false;
    for (unsigned i = 0; i < inst->getNumOperands(); ++i) {
        Value* op = inst->getOperand(i);
        if (auto* opInst = dyn_cast<Instruction>(op)) {
            if (loopBlocks.count(opInst->getParent()))
                return false;
        }
    }
    return true;
}

// 从回边起点 DFS 搜集自然环中的所有块
static void collectLoopBlocks(BasicBlock* header, BasicBlock* back, std::set<BasicBlock*>& loopBlocks) {
    std::stack<BasicBlock*> stack;
    loopBlocks.insert(header);
    loopBlocks.insert(back);
    stack.push(back);

    while (!stack.empty()) {
        BasicBlock* curr = stack.top();
        stack.pop();

        for (BasicBlock* pred : predecessors(curr)) {
            if (!loopBlocks.count(pred)) {
                loopBlocks.insert(pred);
                stack.push(pred);
            }
        }
    }
}

PreservedAnalyses LICM::run(Module& mod, ModuleAnalysisManager&) {
    int moved = 0;
    for (Function& F : mod) {
        if (F.isDeclaration()) continue;

        // 构建反向 DFS 编号，帮助判断回边
        std::unordered_map<BasicBlock*, int> dfsNum;
        std::set<BasicBlock*> visited;
        int counter = 0;

        std::function<void(BasicBlock*)> dfs = [&](BasicBlock* bb) {
            if (visited.count(bb)) return;
            visited.insert(bb);
            dfsNum[bb] = counter++;
            for (BasicBlock* succ : successors(bb)) {
                dfs(succ);
            }
        };
        dfs(&F.getEntryBlock());

        // 寻找回边
        for (BasicBlock& B : F) {
            for (BasicBlock* succ : successors(&B)) {
                if (dfsNum.count(succ) && dfsNum[succ] <= dfsNum[&B]) {
                    // B -> succ 为回边
                    BasicBlock* header = succ;
                    BasicBlock* back = &B;

                    // 找 preheader
                    BasicBlock* preheader = nullptr;
                    for (BasicBlock* pred : predecessors(header)) {
                        if (pred != back)
                            preheader = pred;
                    }
                    if (!preheader) continue;

                    // 收集环中块
                    std::set<BasicBlock*> loopBlocks;
                    collectLoopBlocks(header, back, loopBlocks);

                    // 收集可外提指令
                    std::vector<Instruction*> toMove;
                    for (BasicBlock* bb : loopBlocks) {
                        for (Instruction& I : *bb) {
                            if (isLoopInvariant(&I, loopBlocks)) {
                                toMove.push_back(&I);
                            }
                        }
                    }

                    // 插入 preheader
                    Instruction* insertPt = preheader->getTerminator();
                    for (Instruction* inst : toMove) {
                        inst->moveBefore(insertPt);
                        ++moved;
                    }
                }
            }
        }
    }

    mOut << "LICM running...\nMoved " << moved << " instructions out of loops\n";   
    return PreservedAnalyses::all();
}
