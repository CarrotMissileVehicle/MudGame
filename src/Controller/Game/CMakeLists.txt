# game 库：游戏会话/存档时间模块（Game）。依赖时间服务 time_service。
add_library(game
    src/Game.cpp
)

target_include_directories(game
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(game
    PUBLIC
        time_service
)