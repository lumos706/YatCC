#include <iostream>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/raw_ostream.h>

#include "ConstantFolding.hpp"
#include "Mem2Reg.hpp"
#include "StaticCallCounter.hpp"
#include "StaticCallCounterPrinter.hpp"
#include "ConstantPropagation.hpp"
#include "StrengthReduction.hpp"
#include "AlgebraicIdentities.hpp"
#include "CommonSubexpressionElimination.hpp"
#include "InstructionCombining.hpp"
#include "Inline.hpp"
#include "DeadCodeElimination.hpp"
#include "LoopUnroll.hpp"
#include "LICM.hpp"

#ifdef TASK4_LLM

#include <pybind11/embed.h>

#include "PassSequencePredict.hpp"

namespace Py = pybind11;

#endif

void
opt(llvm::Module& mod)
{
  using namespace llvm;

  // 定义分析pass的管理器
  LoopAnalysisManager lam;
  FunctionAnalysisManager fam;
  CGSCCAnalysisManager cgam;
  ModuleAnalysisManager mam;
  ModulePassManager mpm;

  // 注册分析pass的管理器
  PassBuilder pb;
  pb.registerModuleAnalyses(mam);
  pb.registerCGSCCAnalyses(cgam);
  pb.registerFunctionAnalyses(fam);
  pb.registerLoopAnalyses(lam);
  pb.crossRegisterProxies(lam, fam, cgam, mam);

  // 添加分析pass到管理器中
  mam.registerPass([]() { return StaticCallCounter(); });

#ifdef TASK4_LLM

  // 使用 LLM 技术来辅助编译优化
  // 初始化 Python 解释器
  Py::scoped_interpreter guard{};
  // import sys 库，添加 TASK4_DIR 到寻找 Python 库的 path 中
  Py::module_ sys = Py::module_::import("sys");
  sys.attr("path").attr("append")(TASK4_DIR);

  // 添加 LLM 加持的 Pass 到优化管理器中
  mpm.addPass(PassSequencePredict(
    "sk-WbGDWtXCbr6uqPaJDb0f18Cc81D34d0cA9Fe85541b9564F9",
    "http://505676.proxy.nscc-gz.cn:8888/v1/",
    {
      { "StaticCallCounterPrinter",
        TASK4_DIR "/StaticCallCounterPrinter.hpp",
        TASK4_DIR "/StaticCallCounterPrinter.cpp",
        "StaticCallCounterPrinter.xml",
        [](llvm::ModulePassManager& mpm) {
          mpm.addPass(StaticCallCounterPrinter(llvm::errs()));
        } },
      { "Mem2Reg",
        TASK4_DIR "/Mem2Reg.hpp",
        TASK4_DIR "/Mem2Reg.cpp",
        "Mem2Reg.xml",
        [](llvm::ModulePassManager& mpm) { mpm.addPass(Mem2Reg()); } },
      { "ConstantFolding",
        TASK4_DIR "/ConstantFolding.hpp",
        TASK4_DIR "/ConstantFolding.cpp",
        "ConstantFolding.xml",
        [](llvm::ModulePassManager& mpm) {
          mpm.addPass(ConstantFolding(llvm::errs()));
        } },
      { "ConstantPropagation",
        TASK4_DIR "/ConstantPropagation.hpp",
        TASK4_DIR "/ConstantPropagation.cpp",
        "ConstantPropagation.xml",
        [](llvm::ModulePassManager& mpm) {
          mpm.addPass(ConstantPropagation(llvm::errs()));
        } },
      { "StrengthReduction",
        TASK4_DIR "/StrengthReduction.hpp",
        TASK4_DIR "/StrengthReduction.cpp",
        "StrengthReduction.xml",
        [](llvm::ModulePassManager& mpm) {
          mpm.addPass(StrengthReduction(llvm::errs()));
        } },
      { "AlgebraicIdentities",
        TASK4_DIR "/AlgebraicIdentities.hpp",
        TASK4_DIR "/AlgebraicIdentities.cpp",
        "AlgebraicIdentities.xml",
        [](llvm::ModulePassManager& mpm) {
          mpm.addPass(AlgebraicIdentities(llvm::errs()));
        } },
      { "CommonSubexpressionElimination",
        TASK4_DIR "/CommonSubexpressionElimination.hpp",
        TASK4_DIR "/CommonSubexpressionElimination.cpp",
        "CommonSubexpressionElimination.xml",
        [](llvm::ModulePassManager& mpm) {
          mpm.addPass(CommonSubexpressionElimination(llvm::errs()));
        } },
      { "InstructionCombining",
        TASK4_DIR "/InstructionCombining.hpp",
        TASK4_DIR "/InstructionCombining.cpp",
        "InstructionCombining.xml",
        [](llvm::ModulePassManager& mpm) {
          mpm.addPass(InstructionCombining(llvm::errs()));
        } },
      { "Inline",
        TASK4_DIR "/Inline.hpp",
        TASK4_DIR "/Inline.cpp",
        "Inline.xml",
        [](llvm::ModulePassManager& mpm) {
          mpm.addPass(Inline(llvm::errs()));
        } },
      { "DeadCodeElimination",
        TASK4_DIR "/DeadCodeElimination.hpp",
        TASK4_DIR "/DeadCodeElimination.cpp",
        "DeadCodeElimination.xml",
        [](llvm::ModulePassManager& mpm) {
          mpm.addPass(DeadCodeElimination(llvm::errs()));
        } },
      { "LoopUnroll",
        TASK4_DIR "/LoopUnroll.hpp",
        TASK4_DIR "/LoopUnroll.cpp",
        "LoopUnroll.xml",
        [](llvm::ModulePassManager& mpm) {
          mpm.addPass(LoopUnroll(llvm::errs()));
        } },

    }));

#else

  // 传统 LLVM Pass 来进行编译优化
  // 添加优化pass到管理器中
  mpm.addPass(StaticCallCounterPrinter(llvm::errs()));
  mpm.addPass(Mem2Reg());
  mpm.addPass(ConstantPropagation(llvm::errs()));
  mpm.addPass(ConstantFolding(llvm::errs()));
  mpm.addPass(Inline(llvm::errs()));
  // mpm.addPass(LICM(llvm::errs()));
  
  mpm.addPass(AlgebraicIdentities(llvm::errs()));
  mpm.addPass(CommonSubexpressionElimination(llvm::errs()));
  mpm.addPass(LoopUnroll(llvm::errs()));
  mpm.addPass(InstructionCombining(llvm::errs()));
  mpm.addPass(DeadCodeElimination(llvm::errs()));
  mpm.addPass(StrengthReduction(llvm::errs()));
  // mpm.addPass(DeadCodeElimination(llvm::errs()));


#endif

  // 运行优化pass
  mpm.run(mod, mam);
}

int
main(int argc, char** argv)
{
  if (argc != 3) {
    std::cout << "Usage: " << argv[0] << " <input> <output>\n";
    return -1;
  }

  llvm::LLVMContext ctx;

  llvm::SMDiagnostic err;
  auto mod = llvm::parseIRFile(argv[1], err, ctx);
  if (!mod) {
    std::cout << "Error: unable to parse input file: " << argv[1] << '\n';
    err.print(argv[0], llvm::errs());
    return -2;
  }

  std::error_code ec;
  llvm::StringRef outPath(argv[2]);
  llvm::raw_fd_ostream outFile(outPath, ec);
  if (ec) {
    std::cout << "Error: unable to open output file: " << argv[2] << '\n';
    return -3;
  }

  opt(*mod); // IR的优化发生在这里

  mod->print(outFile, nullptr, false, true);
  if (llvm::verifyModule(*mod, &llvm::outs()))
    return 3;
}
