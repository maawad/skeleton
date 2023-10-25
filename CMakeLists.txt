cmake_minimum_required(VERSION 3.12)

project(skeleton LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(skeleton::skeleton ALIAS skeleton)

option(BUILD_BENCHMARKS "Build benchmarks" ON)
option(BUILD_TESTS "Build tests" ON)
option(BUILD_EXAMPLES "Build examples" ON)

if(BUILD_BENCHMARKS)
   add_subdirectory(benchmarks)
endif(BUILD_BENCHMARKS)

if(BUILD_TESTS)
   add_subdirectory(test)
endif(BUILD_TESTS)

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif(BUILD_EXAMPLES)
