cmake_minimum_required(VERSION 4.0)
project(MudGame)

set(CMAKE_CXX_STANDARD 20)

add_executable(MudGame main.cpp
        src/Controller/Objects/src/Object.cpp
        src/Controller/Objects/include/Object.h
)
