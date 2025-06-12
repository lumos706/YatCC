#include <iostream>
#include <getopt.h>
#include <string>
#include <regex>
#include <sstream>

#include "antlr4-runtime.h"
#include "SysY2022Lexer.h"
#include "MyVisitor.h"
#include "arm.h"
#include "asm_passes.h"

using namespace antlr4;

#ifdef TASK5_LLM

#include <pybind11/embed.h>
#include "IRTrans.hpp"

namespace Py = pybind11;

#endif

int main(int argc, char ** argv) {
  std::ifstream sourceFile(argv[1]);
  assert(sourceFile.is_open());
  
  // Read the entire file into a string
  std::stringstream buffer;
  buffer << sourceFile.rdbuf();
  std::string fileContent = buffer.str();
  sourceFile.close();
  
  std::regex includePattern(R"(#include\s*[<"].*[>"].*\n?)");
  std::string processedContent = std::regex_replace(fileContent, includePattern, "");
  
  ANTLRInputStream input(processedContent);
  SysY2022Lexer lexer(&input);
  CommonTokenStream tokens(&lexer);
  tokens.fill();
  SysY2022Parser parser(&tokens);
  SysY2022Parser::CompUnitContext* tree = parser.compUnit();
  MyVisitor visitor;
  visitor.visitCompUnit(tree);
  visitor.opt();
  visitor.print();

#ifdef TASK5_LLM
  // 初始化Python解释器
  Py::scoped_interpreter guard{};
  // import sys 库，添加 TASK5_DIR 到寻找 Python 库的 path 中
  Py::module_ sys = Py::module_::import("sys");
  sys.attr("path").attr("append")(TASK5_DIR);

  IrTransform::IrTransformInfo mIrTransformInfo;
  mIrTransformInfo.mIRPath = argv[2];
  mIrTransformInfo.mAsmPath = argv[3];
  IrTransform irTrans("<api-key>", "<api-url>", mIrTransformInfo);
  irTrans.Ir2Asm(mIrTransformInfo);
#else
  auto program_asm=emit_asm(visitor.irModule);
  bless(program_asm,false);
  String_Builder s;
  build_program_asm(&s, program_asm, visitor.irModule.globalVariables);
  s.add_terminator();

  FILE* assembly_file = fopen(argv[2], "w");
  if (assembly_file == NULL) {
      assert(false && "error opening assembly output file");
  }

  fprintf(assembly_file, "%s", s.c_str());
  fclose(assembly_file);
#endif
  return 0;
}
