#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Support/raw_ostream.h>
#include <set>
#include <queue>
#include <vector>
#include <map>

class Inline : public llvm::PassInfoMixin<Inline> 
{
public:
  explicit Inline(llvm::raw_ostream& out)
		: mOut(out)
  {
  }
  llvm::PreservedAnalyses run(llvm::Module& mod,
															llvm::ModuleAnalysisManager& mam);

private:
    llvm::raw_ostream& mOut;
};