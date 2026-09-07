cmake_minimum_required(VERSION 4.0)
project(MudGame)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 源码注释为 UTF-8（中文），需显式指定 /utf-8，否则 MSVC 按 GBK(936)
# 解析会在多字节注释末尾吞掉换行导致 C2447/C4819 等错误。
add_compile_options(/utf-8)

include(FetchContent)
FetchContent_Declare(cli11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
    GIT_TAG v2.7.2
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(cli11)

add_subdirectory(src)

# objects 库：游戏物品基类模块（Object），供 mining_controller / cmd_parser / MudGame 链接。
add_library(objects src/model/Objects/src/Object.cpp)
target_include_directories(objects
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/model/Objects/include
)

# Jerry 模块：种菜 / 钓鱼 / 农场 / 肥料 / 市场等完整实现
set(JERRY_SOURCES
        src/Controller/Food/src/Food.cpp
        src/Controller/Crop/src/Crop.cpp
        src/Controller/Crop/src/Cabbage.cpp
        src/Controller/Crop/src/Carrot.cpp
        src/Controller/Crop/src/Tomato.cpp
        src/Controller/Crop/src/Pumpkin.cpp
        src/Controller/Crop/src/Lingzhi.cpp
        src/Controller/Fertilizer/src/Fertilizer.cpp
        src/Controller/Fertilizer/src/NormalFertilizer.cpp
        src/Controller/Fertilizer/src/AdvancedFertilizer.cpp
        src/Controller/Farm/src/Farm.cpp
        src/Controller/Farm/src/FarmLand.cpp
        src/Controller/Fish/src/Fish.cpp
        src/Controller/Fish/src/Crucian.cpp
        src/Controller/Fish/src/GrassCarp.cpp
        src/Controller/Fish/src/Perch.cpp
        src/Controller/Fish/src/RainbowTrout.cpp
        src/Controller/Fish/src/KingCrab.cpp
        src/Controller/Farming/src/FarmingController.cpp
        src/Controller/Fishing/src/FishingController.cpp
        src/Controller/Market/src/Market.cpp
        src/Controller/Market/src/Shop.cpp
        src/Controller/Market/src/ShopItem.cpp
)

add_executable(MudGame main.cpp ${JERRY_SOURCES})
target_link_libraries(MudGame PRIVATE objects mining_controller cmd_parser cli11)

target_include_directories(MudGame PRIVATE
        src/model/Objects/include
        src/model/Time/include
        src/Controller/Food/include
        src/Controller/Crop/include
        src/Controller/Fertilizer/include
        src/Controller/Farm/include
        src/Controller/Fish/include
        src/Controller/Farming/include
        src/Controller/Fishing/include
        src/Controller/Market/include
)
