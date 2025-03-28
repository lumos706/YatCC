file(REAL_PATH ../cases _cases_dir)
file(REAL_PATH ../rtlib _rtlib_dir)

# 生成官方测例表
set(TASK0_CASES_TXT ${CMAKE_CURRENT_BINARY_DIR}/cases.txt)
string(REPLACE ";" "\n" _lines "${TEST_CASES}")
file(WRITE ${TASK0_CASES_TXT} "${_lines}")

# 生成官方测例答案（预处理的结果）
add_custom_target(
  task0-answer
  ${Python3_EXECUTABLE}
  ${CMAKE_CURRENT_SOURCE_DIR}/answer.py
  ${TEST_CASES_DIR} # 测例目录
  ${CMAKE_CURRENT_BINARY_DIR} # 输出目录
  ${TASK0_CASES_TXT} # 测例表
  ${CLANG_EXECUTABLE} # clang 路径
  ${_rtlib_dir} # rtlib 路径
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  USES_TERMINAL
  SOURCES answer.py)

# 对实验进行评分
add_custom_target(
  task0-score
  ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/score.py
  ${CMAKE_CURRENT_BINARY_DIR} $<TARGET_FILE:task0>
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  USES_TERMINAL
  SOURCES score.py)

add_dependencies(task0-score task0)

# ############################################################################ #
# 提供 diy 功能，对单独的cases目录进行重新编译与评分，跳过缓存机制。
# ############################################################################ #

# 生成实验零的 diy-cases 答案
add_custom_target(
  task0-diy-answer
  COMMAND
    ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/answer.py
    ${DIY_TEST_CASES_DIR} # 测例目录
    ${CMAKE_CURRENT_BINARY_DIR}/diy-cases # 输出目录
    ${DIY_TEST_CASES_TXT} # 测例表
    ${CLANG_EXECUTABLE} ${_rtlib_dir} --no-cache # 禁用缓存机制
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  USES_TERMINAL
  SOURCES answer.py)
