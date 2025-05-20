file(REAL_PATH ../rtlib _rtlib_dir)
file(REAL_PATH ../task0 _task0_out BASE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
file(REAL_PATH ../task1 _task1_out BASE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# 生成官方测例表
if(NOT TASK2_CASES_TXT)
  set(_task2_cases "")
  foreach(_case ${TEST_CASES})
    if(NOT _case MATCHES ${TASK2_EXCLUDE_REGEX})
      list(APPEND _task2_cases ${_case})
    endif()
  endforeach()
  set(TASK2_CASES_TXT ${CMAKE_CURRENT_BINARY_DIR}/cases.txt)
  string(REPLACE ";" "\n" _lines "${_task2_cases}")
  file(WRITE ${TASK2_CASES_TXT} "${_lines}")
else()
  exec_program(
    ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/..
    ARGS common.py ${TEST_CASES_DIR} ${CMAKE_CURRENT_BINARY_DIR}
         ${TASK2_CASES_TXT}
    OUTPUT_VARIABLE _task2_cases)
endif()

# 生成官方测例答案
add_custom_target(
  task2-answer
  ${Python3_EXECUTABLE}
  ${CMAKE_CURRENT_SOURCE_DIR}/answer.py
  ${_task0_out} # 输入目录
  ${CMAKE_CURRENT_BINARY_DIR} # 输出目录
  ${TASK2_CASES_TXT} # 测例表
  ${CLANG_EXECUTABLE} # clang 路径
  ${_rtlib_dir} # rtlib 路径
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  USES_TERMINAL
  SOURCES answer.py)

add_dependencies(task2-answer task0-answer)

# 对官方标准实验进行评分
add_custom_target(
  task2-score
  COMMAND
    ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/score.py ${TEST_CASES_DIR}
    ${CMAKE_CURRENT_BINARY_DIR} ${TASK2_CASES_TXT} ${CTEST_COMMAND}
    ${TASK2_LOG_LEVEL}
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  USES_TERMINAL
  SOURCES score.py)

add_dependencies(task2-score task2 task2-answer)

# 为每个官方测例创建一个测试和评分
if(TASK2_REVIVE)
  # 如果启用复活，则将前一个实验的标准答案作为输入
  add_dependencies(task2-score task1-answer)

  foreach(_case ${_task2_cases})
    set(_output_dir ${CMAKE_CURRENT_BINARY_DIR}/${_case})
    file(MAKE_DIRECTORY ${_output_dir})
    add_test(NAME task2/${_case}
             COMMAND task2 ${_task1_out}/${_case}/answer.txt
                     ${_output_dir}/output.json)
    add_test(
      NAME test2/${_case}
      COMMAND
        ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/score.py
        ${TEST_CASES_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${TASK2_CASES_TXT}
        ${CTEST_COMMAND} ${TASK2_LOG_LEVEL} --single ${_case})
  endforeach()

  message(AUTHOR_WARNING "在实验二复活：请在构建 task1-answer 后再使用 task2 的测试项目。")

else()
  # 否则以实验零的标准答案作为输入
  add_dependencies(task2-score task0-answer)

  foreach(_case ${_task2_cases})
    set(_output_dir ${CMAKE_CURRENT_BINARY_DIR}/${_case})
    file(MAKE_DIRECTORY ${_output_dir})
    add_test(NAME task2/${_case} COMMAND task2 ${_task0_out}/${_case}
                                         ${_output_dir}/output.json)
    add_test(
      NAME test2/${_case}
      COMMAND
        ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/score.py
        ${TEST_CASES_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${TASK2_CASES_TXT}
        ${CTEST_COMMAND} ${TASK2_LOG_LEVEL} --single ${_case})
  endforeach()

  message(AUTHOR_WARNING "实验二复活已禁用，请在构建 task0-answer 后再使用 task2 的测试项目。")

endif()

# ############################################################################ #
# 提供 diy 功能，对单独的cases目录进行重新编译与评分，跳过缓存机制。
# ############################################################################ #

# 生成 diy 测例答案
add_custom_target(
  task2-diy-answer
  COMMAND
    ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/answer.py
    ${_task0_out}/diy-cases # 测例目录
    ${CMAKE_CURRENT_BINARY_DIR}/diy-cases # 输出目录
    ${DIY_TEST_CASES_TXT} # 测例表
    ${CLANG_EXECUTABLE} # clang 路径
    ${_rtlib_dir} # rtlib 路径
    --no-cache # 禁用缓存机制
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  USES_TERMINAL
  SOURCES answer.py)

add_dependencies(task2-diy-answer task0-diy-answer)

# 对 diy 测例进行评分
add_custom_target(
  task2-diy-score
  COMMAND
    ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/score.py
    ${DIY_TEST_CASES_DIR} ${CMAKE_CURRENT_BINARY_DIR}/diy-cases
    ${DIY_TEST_CASES_TXT} ${CTEST_COMMAND} ${TASK2_LOG_LEVEL} --diy-mod
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  USES_TERMINAL
  SOURCES score.py)

add_dependencies(task2-diy-score task2 task2-diy-answer)

# 为每个 diy 测例创建一个测试和评分
if(TASK2_REVIVE)
  # 如果启用复活，则将前一个实验的标准答案作为输入
  add_dependencies(task2-diy-score task1-diy-answer)

  foreach(_diy_case ${DIY_TEST_CASES})
    set(_diy_output_dir ${CMAKE_CURRENT_BINARY_DIR}/diy-cases/${_diy_case})
    file(MAKE_DIRECTORY ${_diy_output_dir})
    add_test(NAME task2-diy/${_diy_case}
             COMMAND task2 ${_task1_out}/diy-cases/${_diy_case}/answer.txt
                     ${_diy_output_dir}/output.json)
    add_test(
      NAME test2-diy/${_diy_case}
      COMMAND
        ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/score.py
        ${DIY_TEST_CASES_DIR} ${CMAKE_CURRENT_BINARY_DIR}/diy-cases
        ${DIY_TEST_CASES_TXT} ${CTEST_COMMAND} ${TASK2_LOG_LEVEL} --single
        ${_diy_case})
  endforeach()

  message(AUTHOR_WARNING "在实验二复活：请在构建 task1-diy-answer 后再使用 task2 的 diy 测试项目。")

else()
  # 否则以实验零的标准答案作为输入
  add_dependencies(task2-diy-score task0-diy-answer)

  foreach(_diy_case ${DIY_TEST_CASES})
    set(_diy_output_dir ${CMAKE_CURRENT_BINARY_DIR}/diy-cases/${_diy_case})
    file(MAKE_DIRECTORY ${_diy_output_dir})
    add_test(NAME task2-diy/${_diy_case}
             COMMAND task2 ${_task0_out}/diy-cases/${_diy_case}
                     ${_diy_output_dir}/output.json)
    add_test(
      NAME test2-diy/${_diy_case}
      COMMAND
        ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/score.py
        ${DIY_TEST_CASES_DIR} ${CMAKE_CURRENT_BINARY_DIR}/diy-cases
        ${DIY_TEST_CASES_TXT} ${CTEST_COMMAND} ${TASK2_LOG_LEVEL} --single
        ${_diy_case})
  endforeach()

  message(
    AUTHOR_WARNING "实验二复活已禁用，请在构建 task0-diy-answer 后再使用 task2 的 diy 测试项目。")

endif()
