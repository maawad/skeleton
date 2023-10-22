cmake_minimum_required(VERSION 3.12)

project(skeleton LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(skeleton::skeleton ALIAS skeleton)

add_subdirectory(examples)
