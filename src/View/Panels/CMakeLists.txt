# view_panels 库：各领域面板渲染组件（TimePanel/StatusPanel/MapPanel/FarmPanel/...）。
# 纯 View：接收 DTO 渲染，不查询业务；仅依赖纯枚举/时间类型头（map/playerstates/time_service）。
add_library(view_panels
    src/TimePanel.cpp
    src/StatusPanel.cpp
    src/MapPanel.cpp
    src/FarmPanel.cpp
    src/FishingPanel.cpp
    src/MiningPanel.cpp
    src/MarketPanel.cpp
    src/ToolsPanel.cpp
    src/BlacksmithPanel.cpp
    src/WeatherPanel.cpp
    src/HelpScreen.cpp
    src/MessagePanel.cpp
    src/TerminalView.cpp
)

target_include_directories(view_panels
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(view_panels
    PUBLIC
        view_primitives
        map
        playerstates
)