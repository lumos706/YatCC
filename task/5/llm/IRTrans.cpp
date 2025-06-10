#include "IRTrans.hpp"
#include "LLMHelper.hpp"
#include <filesystem>
#include <functional>
#include <llvm/ADT/StringRef.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Pass.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/raw_ostream.h>
#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>
#include <unordered_map>
namespace Fs = std::filesystem;
namespace Py = pybind11;
using namespace Py::literals;
using Role = LLMHelper::Role;

namespace {
std::string
read_file(llvm::StringRef filePath)
{
  auto inFileOrErr = llvm::MemoryBuffer::getFile(filePath);
  if (auto err = inFileOrErr.getError()) {
    llvm::errs() << "无法读取文件：" << filePath << '\n';
    std::abort();
  }
  return std::move(inFileOrErr.get()->getBuffer().str());
}

void
write_file(llvm::StringRef filePath, llvm::StringRef content)
{
  std::error_code ec;
  llvm::raw_fd_ostream outFile(filePath, ec);
  if (ec) {
    llvm::errs() << "无法打开文件：" << filePath << '\n';
    std::abort();
  }

  outFile << content;
}
} // namespace


IrTransform::IrTransform(
  llvm::StringRef apiKey,
  llvm::StringRef baseURL,
  IrTransformInfo mInfo)
  : mHelper(apiKey, baseURL)
  , mIrTransformInfo(mInfo)
{
}

std::string
IrTransform::Ir2Asm(IrTransform::IrTransformInfo& mirTransformInfo)
{
    // 对比缓存内容、头文件和实现文件的最后修改时间
    if (Fs::exists(mirTransformInfo.mAsmPath)) {
        auto asmLastWriteTime = Fs::last_write_time(mirTransformInfo.mAsmPath);
        auto irLastWriteTime = Fs::last_write_time(mirTransformInfo.mIRPath);

        if (asmLastWriteTime >= irLastWriteTime) {
            return read_file(mirTransformInfo.mAsmPath);
        }
    }
    std::string systemPrompt =
    read_file(TASK5_DIR "/llm/prompts/Ir2AsmSysPrTpl.xml");
    auto userPrompt =
        Py::str(read_file(TASK5_DIR "/llm/prompts/Ir2AsmUserPrTpl.xml"))
        .attr("format")("ir"_a = read_file(mirTransformInfo.mIRPath))
        .cast<std::string>();

    // 创建会话
    std::string sessionID = mHelper.create_new_session();
    mHelper.add_content(sessionID, Role::kSystem, systemPrompt);
    mHelper.add_content(sessionID, Role::kUser, userPrompt);

    Py::module_ llm = Py::module_::import("llm");
    Py::list handlers;
    handlers.append(llm.attr("remove_deepseek_r1_think"));
    handlers.append(llm.attr("remove_md_block_marker")("xml"));
    std::string response = mHelper.chat(
        sessionID,
        "deepseek-r1",
        handlers,
        Py::dict("max_tokens"_a = 8192, "stream"_a = false, "temperature"_a = 0));

    write_file(mirTransformInfo.mAsmPath, response);

    // 删除会话
    mHelper.delete_session(sessionID);
    return response;
}
