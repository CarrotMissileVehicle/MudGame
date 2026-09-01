# mining_controller 库：采矿领域控制器与数据模块。
add_library(mining_controller
    src/mining_controller.cpp
    src/mining_calculator.cpp
    src/mining_state.cpp
    src/ore_data.cpp
    src/ore_layer.cpp
)

# 依赖物品基类(objects)与时间服务(time_service)。
target_link_libraries(mining_controller
    PUBLIC
        objects
        time_service
)

# 对外暴露本模块头文件目录。
target_include_directories(mining_controller
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
