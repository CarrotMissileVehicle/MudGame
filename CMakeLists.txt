cmake_minimum_required(VERSION 4.0)
project(MudGame)

set(CMAKE_CXX_STANDARD 20)

add_executable(MudGame main.cpp
        Controller/include/PlayerStateMachine.h
        PlayerStates/src/PlayerState.cpp
        PlayerStates/include/PlayerState.h
        PlayerStates/include/PlayerStateCode.h
        Map/src/Position.cpp
        Map/include/Position.h
        Map/src/Town.cpp
        Map/include/Town.h
        Map/src/Home.cpp
        Map/include/Home.h
        Map/src/Mine.cpp
        Map/include/Mine.h
        Map/src/Farmland.cpp
        Map/include/Farmland.h
        Map/src/Coast.cpp
        Map/include/Coast.h
        Map/include/PositionCode.h
        Player/Player.cpp
        Player/Player.h
)
