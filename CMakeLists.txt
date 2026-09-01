cmake_minimum_required(VERSION 4.0)
project(MudGame)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# CLI11: header-only 命令行解析库，用于命令解析（cmd_parser）
# 首次拉取后会生成 CLI11::CLI11 目标，其 include 路径位于构建目录 _deps 下，
# 需在 CLion 中 Reload CMake Project 后 IDE 才能解析到 <CLI/CLI.hpp>。
include(FetchContent)
FetchContent_Declare(cli11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
    GIT_TAG v2.7.2
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(cli11)

add_subdirectory(src)

add_executable(MudGame main.cpp)
target_link_libraries(MudGame PRIVATE objects mining_controller cmd_parser)