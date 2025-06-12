# 获取实时库路径
file(REAL_PATH ../rtlib _rtlib_dir)
# 获取task0和task3的输出目录路径
file(REAL_PATH ../task5 _task5_out BASE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
file(REAL_PATH ../cases _task5_in BASE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

set(GCC_EXE /usr/bin/arm-linux-gnueabihf-gcc)
set(YATCC_EXE ${CMAKE_CURRENT_SOURCE_DIR}/../bin/YatCC)
set(TASK5_RUNTIME ${CMAKE_CURRENT_SOURCE_DIR}/libsysy.a)
set(QENU /usr/bin/qemu-arm-static)

# 生成测例表(Generate test case table)
if(NOT TASK5_CASES_TXT)
  # 如果未指定测例列表文件，则根据测试排除规则筛选测例
  set(_task5_cases "")
  foreach(_case ${TEST_CASES})
    if(NOT _case MATCHES ${TASK5_EXCLUDE_REGEX})
      list(APPEND _task5_cases ${_case})
    endif()
  endforeach()
  # 将筛选后的测例列表写入cases.txt文件
  set(TASK5_CASES_TXT ${CMAKE_CURRENT_BINARY_DIR}/cases.txt)
  string(REPLACE ";" "\n" _lines "${_task5_cases}")
  file(WRITE ${TASK5_CASES_TXT} "${_lines}")
else()
  # 如果已指定测例列表文件，则使用Python脚本处理测例
  exec_program(
    ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/..
    ARGS common.py ${TEST_CASES_DIR} ${CMAKE_CURRENT_BINARY_DIR}
         ${TASK5_CASES_TXT}
    OUTPUT_VARIABLE _task5_cases)
endif()

# 生成全部答案(Generate all answers)
add_custom_target(
  task5-answer
  ${Python3_EXECUTABLE}
  ${CMAKE_CURRENT_SOURCE_DIR}/answer.py
  ${TEST_CASES_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${TASK5_CASES_TXT}
  ${GCC_EXE}
  ${YATCC_EXE}
  ${QENU}
  ${_rtlib_dir}
  ${TASK5_RUNTIME}
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  USES_TERMINAL
  SOURCES answer.py)

# 设置依赖关系：task5的答案依赖于运行时库
add_dependencies(task5-answer test-rtlib)

message(STATUS "TEST_RTLIB_SO = ${TEST_RTLIB_SO}")
# 对实验进行评分(Score the experiment)
add_custom_target(
  task5-score
  COMMAND
    ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/score.py ${TEST_CASES_DIR}
    ${CMAKE_CURRENT_BINARY_DIR} ${TASK5_CASES_TXT} ${CTEST_COMMAND}
    ${GCC_EXE} ${QENU} ${TASK5_RUNTIME}
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  USES_TERMINAL
  SOURCES score.py)

# 设置评分依赖关系：评分依赖于task4实现、答案生成和运行时库
add_dependencies(task5-score task5 task5-answer test-rtlib)


foreach(_case ${_task5_cases})
  set(_output_dir ${CMAKE_CURRENT_BINARY_DIR}/${_case})
  file(MAKE_DIRECTORY ${_output_dir})
  if(_case MATCHES ${TASK5_LLM_REGEX})
    # 对于匹配LLM正则表达式的测例使用task5-llm命令
    add_test(NAME task5/${_case}
            COMMAND bash -c "${CMAKE_BINARY_DIR}/task/5/task5-llm \
            '${_task5_in}/${_case}' '${_task5_out}/${_case}/output.ll' \
            '${_task5_out}/${_case}/output.s' \
            > '${_task5_out}/${_case}/output.ll'")
  else()
    # 对于其他测例使用task5-classic命令
    add_test(NAME task5/${_case}
            COMMAND bash -c "${CMAKE_BINARY_DIR}/task/5/task5-classic \
            '${_task5_in}/${_case}' '${_task5_out}/${_case}/output.s' \
            > '${_task5_out}/${_case}/output.ll'")
  endif()
  # 为每个测例添加评分测试
  add_test(
    NAME test5/${_case}
    COMMAND
      ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/score.py
      ${TEST_CASES_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${TASK5_CASES_TXT}
      ${CTEST_COMMAND} ${GCC_EXE} ${QENU} ${TASK5_RUNTIME} --single
      ${_case})
endforeach()

message(AUTHOR_WARNING "在实验五默认复活")