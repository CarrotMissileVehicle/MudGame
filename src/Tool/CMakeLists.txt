# player_serializer 库：玩家存档/读档模块（src/Tool）。依赖 game、player 与 tool_controller。
add_library(player_serializer
    src/PlayerSerializer.cpp
)

target_include_directories(player_serializer
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(player_serializer
    PUBLIC
        game
        player
        tool_controller
)