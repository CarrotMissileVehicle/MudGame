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

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(cli11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
    GIT_TAG v2.7.2
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(cli11)

# pjh_json：矿石/矿区 JSON 数据解析库。关闭其激进优化
# (-march=native/-ffast-math/-flto) 传播，避免污染全工程。
set(PJH_JSON_ENABLE_OPTIMIZATIONS OFF CACHE BOOL "Disable pjh_json aggressive opts" FORCE)
FetchContent_Declare(pjh_json
    GIT_REPOSITORY https://github.com/pjh456/pjh_json.git
    GIT_TAG main
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(pjh_json)

add_subdirectory(src)

# 最终入口 main.cpp 暂未编写，先移出可执行目标以保证库构建可用；
# 编写入口后恢复以下两行即可。
# add_executable(MudGame main.cpp)
# target_link_libraries(MudGame PRIVATE objects mining_controller cmd_parser)