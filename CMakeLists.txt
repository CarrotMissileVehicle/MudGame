# =====================================================================
# MudGame 顶层 CMake 构建脚本
# ---------------------------------------------------------------------
# 职责：
#   1. 声明 C++23 标准与构建要求
#   2. 通过 FetchContent 拉取头文件库 CLI11（用于命令行解析）
#   3. 引入 src 子目录并构建可执行目标 MudGame
#
# =====================================================================
cmake_minimum_required(VERSION 4.0)
project(MudGame)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(cli11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
    GIT_TAG v2.7.2
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(cli11)

add_subdirectory(src)

add_executable(MudGame main.cpp)
target_link_libraries(MudGame PRIVATE objects mining_controller cmd_parser)