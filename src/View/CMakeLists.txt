# View 层公共渲染原语库：Renderer + 通用布局原语 + DTO 头（dto.h / Renderer.h / view_primitives.h）。
# 只依赖纯时间类型（time_service），不触碰任何业务模块。
add_library(view_primitives
    src/Renderer.cpp
    src/view_primitives.cpp
)

target_include_directories(view_primitives
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(view_primitives
    PUBLIC
        time_service
)

# ---- 子视图模块：面板 / 命令解析 / FTXUI 终端界面 ----
add_subdirectory(Panels)
add_subdirectory(Cmdparser)
add_subdirectory(Tui)