cmake_minimum_required(VERSION 4.0)
project(MudGame)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 公共源文件
set(MODEL_SOURCES
        src/model/Objects/src/Object.cpp
        src/model/PlayerStates/src/PlayerState.cpp
        src/model/Map/src/Position.cpp
        src/model/Map/src/Town.cpp
        src/model/Map/src/Home.cpp
        src/model/Map/src/Mine.cpp
        src/model/Map/src/Farmland.cpp
        src/model/Map/src/Coast.cpp
        src/model/Bag/src/Bag.cpp
        src/controller/Player/include/Move.h
)

set(CONTROLLER_SOURCES
        src/controller/Player/src/Player.cpp
        src/controller/Player/src/Move.cpp
        src/controller/Player/include/Move.h
        src/controller/Game/src/Game.cpp
        src/controller/Game/include/Game.h
)

set(VIEW_SOURCES
        src/view/src/StatusView.cpp
        src/controller/Player/include/Move.h
        src/model/Time/include/Time.h
        src/model/Time/src/Time.cpp
)

set(TOOL_SOURCES
        src/tool/src/PlayerSerializer.cpp
        src/controller/Player/include/Move.h
)

# 主程序
add_executable(MudGame main.cpp
        ${MODEL_SOURCES}
        ${CONTROLLER_SOURCES}
        ${VIEW_SOURCES}
        ${TOOL_SOURCES}
        src/controller/Player/include/Move.h
)
add_subdirectory(src)

target_link_libraries(MudGame PRIVATE objects mining_controller cmd_parser cmd_game)
# 测试程序
add_executable(MudGameTest test/test_player.cpp
        ${MODEL_SOURCES}
        ${CONTROLLER_SOURCES}
        ${TOOL_SOURCES}
        src/controller/Player/include/Move.h
)

target_include_directories(MudGame PRIVATE src)
target_include_directories(MudGameTest PRIVATE src)
