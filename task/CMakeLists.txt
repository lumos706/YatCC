find_package(FLEX 2.6)
find_package(BISON 3.8)

find_package(antlr4-runtime 4.13)
find_package(antlr4-generator 4.13)
find_package(pybind11 2.13.6)

if(NOT EXISTS ${LLVM_INSTALL_DIR})
  message(FATAL_ERROR "未找到 LLVM 安装目录！请检查 YatCC_LLVM_DIR 环境变量。")
endif()
add_library(LLVM INTERFACE IMPORTED)
target_include_directories(LLVM SYSTEM AFTER INTERFACE
  ${LLVM_INSTALL_DIR}/include)
target_link_directories(LLVM INTERFACE ${LLVM_INSTALL_DIR}/lib)
target_compile_definitions(LLVM INTERFACE
  -D_GNU_SOURCE -D_DEBUG -D_GLIBCXX_ASSERTIONS -D__STDC_CONSTANT_MACROS)
target_link_libraries(LLVM INTERFACE libLLVM.so)

macro(add_task task)
  if(NOT STUDENT_ID STREQUAL "" OR NOT STUDENT_NAME STREQUAL "")
    configure_file(config-${task}.cmake.in
                   ${CMAKE_CURRENT_SOURCE_DIR}/${task}/config.cmake @ONLY)
  endif()
  add_subdirectory(${task})

  set(_student "${STUDENT_ID}-${STUDENT_NAME}")
  set(_out "${CMAKE_CURRENT_BINARY_DIR}/task${task}.${_student}.tar.xz")
  set(_msg "作业已打包，请提交 '${_out}' 到评测服务器。")

  add_custom_target(
    task${task}-pack
    COMMAND ${CMAKE_COMMAND} -E tar cvfJ ${_out}
            ${CMAKE_CURRENT_SOURCE_DIR}/${task}
    COMMAND echo ${_msg}
    COMMAND echo ${_msg}
    COMMAND echo ${_msg}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endmacro()

add_task(0)
add_task(1)
add_task(2)
add_task(3)
add_task(4)
add_task(5)
