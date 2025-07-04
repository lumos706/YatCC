#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Support/raw_ostream.h>
#include <unordered_map>
#include <map>
#include <tuple>

class CommonSubexpressionElimination : public llvm::PassInfoMixin<CommonSubexpressionElimination>
{
public:
  explicit CommonSubexpressionElimination(llvm::raw_ostream& out)
    : mOut(out)
  {
  }

  llvm::PreservedAnalyses run(llvm::Module& mod,
                              llvm::ModuleAnalysisManager& mam);

private:
  llvm::raw_ostream& mOut;
};
