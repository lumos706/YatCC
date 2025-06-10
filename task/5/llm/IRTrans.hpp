#pragma once

#include "LLMHelper.hpp"
#include <functional>
#include <initializer_list>
#include <llvm/IR/PassManager.h>
#include <string>

class IrTransform
{
public:
  struct IrTransformInfo
  {
    std::string mIRPath;
    std::string mAsmPath;
  };

  IrTransform(llvm::StringRef apiKey,
                      llvm::StringRef baseURL,
                      IrTransformInfo mInfo);
  
  std::string Ir2Asm(IrTransformInfo& mIrTransformInfo);

private:
  LLMHelper mHelper;
  IrTransformInfo mIrTransformInfo;
};
