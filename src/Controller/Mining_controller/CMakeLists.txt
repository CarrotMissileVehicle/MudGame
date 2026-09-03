# mining_controller 库：采矿领域控制器与数据模块。
add_library(mining_controller
    src/mining_controller.cpp
    src/mining_calculator.cpp
    src/mining_state.cpp
    src/ore_data.cpp
    src/ore_layer.cpp
)

# 依赖物品基类(objects)与时间服务(time_service)，及 pjh_json 解析库。
target_link_libraries(mining_controller
    PUBLIC
        objects
        time_service
        pjh::json
)

# 对外暴露本模块头文件目录。
target_include_directories(mining_controller
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 注入游戏数据目录（src/Data），供 Ore/Layer 定位 JSON 数据文件。
set(MUDGAME_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../Data")
file(TO_CMAKE_PATH ${MUDGAME_DATA_DIR} MUDGAME_DATA_DIR)
target_compile_definitions(mining_controller
    PUBLIC
        MUDGAME_DATA_DIR="${MUDGAME_DATA_DIR}"
)
