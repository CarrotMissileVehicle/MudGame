cmake_minimum_required(VERSION 4.0)
project(MudGame)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(src)

add_executable(MudGame main.cpp)
target_link_libraries(MudGame PRIVATE objects mining_controller cmd_parser cmd_game)