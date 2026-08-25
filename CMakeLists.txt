cmake_minimum_required(VERSION 4.0)
project(MudGame)

set(CMAKE_CXX_STANDARD 20)

add_executable(MudGame main.cpp
        Controller/include/PlayerStateMachine.h
        PlayerStates/src/PlayerState.cpp
        PlayerStates/include/PlayerState.h
        PlayerStates/include/PlayerStateCode.h
        Map/Position.cpp
        Map/Position.h
        Map/Town.cpp
        Map/Town.h
        Map/Home.cpp
        Map/Home.h
        Map/Mine.cpp
        Map/Mine.h
        Map/Farmland.cpp
        Map/Farmland.h
        Map/Coast.cpp
        Map/Coast.h
        Map/PositionCode.h
)
