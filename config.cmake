# 你的学号
set(STUDENT_ID "22336216")
# 你的姓名
set(STUDENT_NAME "陶宇卓")

# 实验一的完成方式："flex"或"antlr"
set(TASK1_WITH "antlr")
# 实验一的日志级别，级别从低到高为0-3
set(TASK1_LOG_LEVEL 3)

# 实验二的完成方式："bison"或"antlr"
set(TASK2_WITH "bison")
# 是否在实验二复活，ON或OFF
set(TASK2_REVIVE ON)
# 实验二的日志级别，级别从低到高为0-3
set(TASK2_LOG_LEVEL 3)

# 是否在实验三复活，ON或OFF
set(TASK3_REVIVE ON)

# 是否在实验四复活，ON或OFF
set(TASK4_REVIVE ON)

# 是否在实验五复活，ON或OFF
set(TASK5_REVIVE ON)

# ############################################################################ #
# 以下内容为内部环境配置，一般情况下不需要学生修改，学生本地的修改对实验评测无影响。
# ############################################################################ #

# ANTLR4
if(DEFINED ENV{YatCC_ANTLR_DIR})
  set(_antlr_dir "$ENV{YatCC_ANTLR_DIR}")
else()
  set(_antlr_dir "${CMAKE_SOURCE_DIR}/antlr")
endif()
message("ANTLR目录为 ${_antlr_dir}")
set(antlr4-runtime_DIR "${_antlr_dir}/install/lib/cmake/antlr4-runtime")
set(antlr4-generator_DIR "${_antlr_dir}/install/lib/cmake/antlr4-generator")
set(ANTLR4_JAR_LOCATION "${_antlr_dir}/antlr.jar")

# LLVM 和 Clang
if(DEFINED ENV{YatCC_LLVM_DIR})
  set(_llvm_dir "$ENV{YatCC_LLVM_DIR}")
else()
  set(_llvm_dir "${CMAKE_SOURCE_DIR}/llvm")
endif()
message("LLVM目录为 ${_llvm_dir}")
set(LLVM_DIR "${_llvm_dir}/install/lib/cmake/llvm")
set(LLVM_INSTALL_DIR "${_llvm_dir}/install")
set(CLANG_EXECUTABLE "${_llvm_dir}/install/bin/clang")
set(CLANG_PLUS_EXECUTABLE "${_llvm_dir}/install/bin/clang++")

# PYBIND11
if(DEFINED ENV{YatCC_PYBIND11_DIR})
  set(_pybind11_dir "$ENV{YatCC_PYBIND11_DIR}")
else()
  set(_pybind11_dir "${CMAKE_SOURCE_DIR}/pybind11")
endif()
message("PYBIND11目录为 ${_pybind11_dir}")
set(pybind11_DIR "${_pybind11_dir}/install/share/cmake/pybind11")

# 测试运行时限（秒）
set(CTEST_TEST_TIMEOUT 3)

# 实验一排除测例名的正则式
set(TASK1_EXCLUDE_REGEX "^performance/.*|^llm-performance/.*|^llm-backend/.*")
# 实验一测例表，非空时忽略 EXCLUDE_REGEX
set(TASK1_CASES_TXT "")

# 实验二排除测例名的正则式
set(TASK2_EXCLUDE_REGEX "^performance/.*|^llm-performance/.*|^llm-backend/.*")
# 实验二测例表，非空时忽略 EXCLUDE_REGEX
set(TASK2_CASES_TXT "")

# 实验三排除测例名的正则式
set(TASK3_EXCLUDE_REGEX "^performance/.*|^llm-performance/.*|^llm-backend/.*")
# 实验三测例表，非空时忽略 EXCLUDE_REGEX
set(TASK3_CASES_TXT "")

# 实验四排除测例名的正则式
set(TASK4_EXCLUDE_REGEX "^functional-.*|^mini-performance/.*|^llm-backend/.*")
# 实验四 LLM 定向优化测例名正则式
set(TASK4_LLM_REGEX "^llm-performance/.*")
# 实验四测例表，非空时忽略 EXCLUDE_REGEX
set(TASK4_CASES_TXT "")

# 实验五排除测例名的正则式
set(TASK5_EXCLUDE_REGEX "^performance/.*|^mini-performance/.*|^llm-performance/.*")
# 实验五 LLM 定向优化测例名正则式
set(TASK5_LLM_REGEX "^llm-backend/.*")
# 实验五测例表，非空时忽略 EXCLUDE_REGEX
set(TASK5_CASES_TXT "")
