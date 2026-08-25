cmake_minimum_required(VERSION 4.0)
project(MudGame)

set(CMAKE_CXX_STANDARD 20)

add_executable(MudGame main.cpp
        Objects/include/Object.cpp
        Objects/src/Object.h
)
