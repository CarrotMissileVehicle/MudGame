# 项目仅使用基础 CMake 特性（add_subdirectory / add_library / add_executable
#  / target_include_directories / target_compile_options / CXX_STANDARD），
# 均远早于 4.0 即已可用；本机工具链为 VS 内置 CMake 3.31.6，故降到 3.16 兼容。
cmake_minimum_required(VERSION 3.16)
project(MudGame)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(src)

add_executable(MudGame main.cpp)
target_link_libraries(MudGame PRIVATE objects mining_controller cmd_parser cmd_game)