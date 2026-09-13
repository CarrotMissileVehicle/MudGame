# view_tui 库：基于 FTXUI 的终端图形界面宿主。
# 布局：左上日志区 | 底部输入框 | 输入框上方动态完成列表 + 问题提示行。
add_library(view_tui
    src/GameTui.cpp
    src/LogView.cpp
    src/TuiRenderer.cpp
)

target_include_directories(view_tui
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(view_tui
    PUBLIC
        view_primitives
        ftxui::component
        ftxui::dom
        ftxui::screen
)
