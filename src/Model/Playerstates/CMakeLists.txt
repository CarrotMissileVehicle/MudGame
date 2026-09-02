# playerstates 库：玩家状态模型。依赖地图（PositionCode）。
add_library(playerstates
    src/PlayerState.cpp
)

target_include_directories(playerstates
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(playerstates
    PUBLIC
        map
)