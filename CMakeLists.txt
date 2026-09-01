cmake_minimum_required(VERSION 4.0)
project(MudGame)

set(CMAKE_CXX_STANDARD 20)

add_executable(MudGame main.cpp
        src/Controller/Objects/src/Object.cpp
        src/Controller/Objects/include/Object.h
        src/Controller/PlayerStates/src/PlayerState.cpp
        src/Controller/PlayerStates/include/PlayerState.h
        src/Controller/PlayerStates/include/PlayerStateCode.h
        src/Controller/Map/src/Position.cpp
        src/Controller/Map/include/Position.h
        src/Controller/Map/src/Town.cpp
        src/Controller/Map/include/Town.h
        src/Controller/Map/src/Home.cpp
        src/Controller/Map/include/Home.h
        src/Controller/Map/src/Mine.cpp
        src/Controller/Map/include/Mine.h
        src/Controller/Map/src/Farmland.cpp
        src/Controller/Map/include/Farmland.h
        src/Controller/Map/src/Coast.cpp
        src/Controller/Map/include/Coast.h
        src/Controller/Map/include/PositionCode.h
        src/Controller/Player/Player.cpp
        src/Controller/Player/Player.h
        src/Controller/Bag/src/Bag.cpp
        src/Controller/Bag/include/Bag.h
        src/Controller/Bag/include/Bag.h
        src/Controller/Bag/include/Bag.h
        src/Controller/Map/include/Position.h
)
