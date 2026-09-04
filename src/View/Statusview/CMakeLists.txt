# statusview 库：玩家状态栏显示模块（View/Statusview）。
# 依赖地图 PositionCode 与玩家状态 PlayerStateCode。
add_library(statusview
    src/StatusView.cpp
)

target_include_directories(statusview
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(statusview
    PUBLIC
        map
        playerstates
)