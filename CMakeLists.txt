cmake_minimum_required(VERSION 4.0)
project(MudGame)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(MudGame main.cpp
        src/objects/src/object.cpp
        src/objects/include/object.h
)

add_subdirectory(src/objects)
add_subdirectory(src/controller/mining_controller)