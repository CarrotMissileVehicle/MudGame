cmake_minimum_required(VERSION 4.0)
project(MudGame)

set(CMAKE_CXX_STANDARD 20)

add_executable(MudGame main.cpp
        src/model/Objects/src/Object.cpp
        src/model/Objects/include/Object.h
        src/model/PlayerStates/src/PlayerState.cpp
        src/model/PlayerStates/include/PlayerState.h
        src/model/PlayerStates/include/PlayerStateCode.h
        src/model/Map/src/Position.cpp
        src/model/Map/include/Position.h
        src/model/Map/src/Town.cpp
        src/model/Map/include/Town.h
        src/model/Map/src/Home.cpp
        src/model/Map/include/Home.h
        src/model/Map/src/Mine.cpp
        src/model/Map/include/Mine.h
        src/model/Map/src/Farmland.cpp
        src/model/Map/include/Farmland.h
        src/model/Map/src/Coast.cpp
        src/model/Map/include/Coast.h
        src/model/Map/include/PositionCode.h
        src/Controller/Player/Player.cpp
        src/Controller/Player/Player.h
        src/model/Bag/src/Bag.cpp
        src/model/Bag/include/Bag.h
        src/model/Bag/include/Bag.h
        src/model/Bag/include/Bag.h
        src/model/Map/include/Position.h
        src/view/src/StatusView.cpp
        src/view/include/StatusView.h
        src/tool/src/PlayerSerializer.cpp
        src/tool/include/PlayerSerializer.h
)
