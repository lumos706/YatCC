include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/frontEnd
    ${CMAKE_CURRENT_SOURCE_DIR}/backEnd
    ${CMAKE_CURRENT_SOURCE_DIR}/middleEnd
    ${CMAKE_CURRENT_SOURCE_DIR}/opt
    ${CMAKE_CURRENT_SOURCE_DIR}/backEnd/asm
    ${ANTLR4_INCLUDE_DIRS}
)

file(GLOB_RECURSE ALL_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)
list(FILTER ALL_SOURCES EXCLUDE REGEX ".*llm/.*")
set(_classic_src ${ALL_SOURCES})

message(STATUS "Found source files:")
foreach(SOURCE_FILE ${_classic_src})
    message(STATUS "  ${SOURCE_FILE}")
endforeach()

add_executable(task5-classic ${_classic_src})

target_include_directories(task5-classic PRIVATE . ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(task5-classic PRIVATE ${LLVM_INCLUDE_DIRS})

target_link_libraries(task5-classic antlr4_static)
target_link_libraries(task5-classic LLVM)

file(GLOB_RECURSE _llm_src 
    "${CMAKE_CURRENT_SOURCE_DIR}/llm/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/llm/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/llm/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/llm/*.hpp"
)

add_executable(task5-llm ${_llm_src} ${_classic_src})
target_include_directories(task5-llm PRIVATE . ${CMAKE_CURRENT_BINARY_DIR} llm)
target_include_directories(task5-llm PRIVATE ${LLVM_INCLUDE_DIRS})
target_link_libraries(task5-llm LLVM pybind11::embed antlr4_static)

target_compile_definitions(task5-llm
                           PRIVATE TASK5_LLM TASK5_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

add_custom_target(task5 DEPENDS task5-classic task5-llm)
