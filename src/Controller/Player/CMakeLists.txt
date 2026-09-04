# player 库：玩家控制器。依赖背包 bag、地图 map、状态 playerstates。
add_library(player
    src/Player.cpp
    src/Move.cpp
)

target_include_directories(player
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(player
    PUBLIC
        bag
        map
        playerstates
)