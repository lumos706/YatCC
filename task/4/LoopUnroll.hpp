#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Support/raw_ostream.h>
#include <vector>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/CFG.h>
#include <set>

class LoopUnroll : public llvm::PassInfoMixin<LoopUnroll> {
public:
    LoopUnroll(llvm::raw_ostream& out) : mOut(out) {}
    llvm::PreservedAnalyses run(llvm::Module& mod, llvm::ModuleAnalysisManager& mam);
private:
    llvm::raw_ostream& mOut;
};