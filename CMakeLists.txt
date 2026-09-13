# =====================================================================
# MudGame 顶层 CMake 构建脚本
# ---------------------------------------------------------------------
# 职责：
#   1. 声明 C++23 标准与构建要求
#   2. 通过 FetchContent 拉取头文件库 CLI11（用于命令行解析）
#   3. 引入 src 子目录并构建可执行目标 MudGame
#
# =====================================================================
cmake_minimum_required(VERSION 4.0)
project(MudGame)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 源码注释为 UTF-8（中文），需显式指定 /utf-8，否则 MSVC 按 GBK(936)
# 解析会在多字节注释末尾吞掉换行导致 C2447/C4819 等错误。
if(MSVC)
    add_compile_options(/utf-8)
endif()

include(FetchContent)
FetchContent_Declare(cli11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
    GIT_TAG v2.7.2
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(cli11)

# pjh_json：矿石/矿区 JSON 数据解析库。关闭其激进优化
# (-march=native/-ffast-math/-flto) 传播，避免污染全工程。
set(PJH_JSON_ENABLE_OPTIMIZATIONS OFF CACHE BOOL "Disable pjh_json aggressive opts" FORCE)
FetchContent_Declare(pjh_json
    GIT_REPOSITORY https://github.com/pjh456/pjh_json.git
    GIT_TAG main
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(pjh_json)

# FTXUI：GitHub 上最受欢迎的 C++ TUI 库（10k+ stars，v7.0.3）。
# 原仓库：https://github.com/ArthurSonzogni/FTXUI.git
# 说明：当前网络无法直连 GitHub，故改用 Gitee 官方镜像镜像仓库（与 GitHub 内容/标签一致）。
# 用于构建新的终端图形界面（日志区 / 输入框 / 动态完成列表 / 问题提示行）。
FetchContent_Declare(ftxui
    GIT_REPOSITORY https://gitee.com/mirrors/ftxui.git
    GIT_TAG v7.0.3
    GIT_SHALLOW TRUE)
set(FTXUI_BUILD_EXAMPLES OFF CACHE BOOL "Disable FTXUI examples" FORCE)
set(FTXUI_BUILD_DOCS OFF CACHE BOOL "Disable FTXUI docs" FORCE)
set(FTXUI_BUILD_TESTS OFF CACHE BOOL "Disable FTXUI tests" FORCE)
FetchContent_MakeAvailable(ftxui)

add_subdirectory(src)

# ---- 单元测试（CTest 开关控制，默认开启）----
include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

# model_objects 库：游戏物品基类模块（Object）在 src/model/Objects/CMakeLists.txt 定义，
# 供 mining_controller / cmd_parser / Bag / MudGame 链接。

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
        src/Controller/Market/src/MarketController.cpp
        src/Controller/Market/src/Shop.cpp
        src/Controller/Market/src/ShopItem.cpp
)

add_executable(MudGame main.cpp ${JERRY_SOURCES})
target_link_libraries(MudGame PRIVATE
        model_objects
        mining_controller
        cmd_parser
        player
        game
        weather_controller
        tool_controller
        player_serializer
        view_panels
        view_tui
        audio
        CLI11::CLI11
        ftxui::component
        ftxui::dom
        ftxui::screen)

target_include_directories(MudGame PRIVATE
        src/Controller/Food/include
        src/Controller/Crop/include
        src/Controller/Fertilizer/include
        src/Controller/Farm/include
        src/Controller/Fish/include
        src/Controller/Farming/include
        src/Controller/Fishing/include
        src/Controller/Market/include
)

# 注入资源目录（res/），供 main 定位背景音乐文件（模拟 MUDGAME_DATA_DIR 的做法）。
set(MUDGAME_RES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/res")
file(TO_CMAKE_PATH ${MUDGAME_RES_DIR} MUDGAME_RES_DIR)
target_compile_definitions(MudGame PRIVATE
    MUDGAME_RES_DIR="${MUDGAME_RES_DIR}"
)
