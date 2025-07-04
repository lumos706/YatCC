#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Support/raw_ostream.h>
#include <set>
#include <queue>
#include <vector>
#include <map>

class DeadCodeElimination : public llvm::PassInfoMixin<DeadCodeElimination> 
{
public:
  explicit DeadCodeElimination(llvm::raw_ostream& out)
		: mOut(out)
  {
  }
  llvm::PreservedAnalyses run(llvm::Module& mod,
								llvm::ModuleAnalysisManager& mam);

private:
    llvm::raw_ostream& mOut;
};