# weather_controller 库：天气与随机事件系统（Model 层 + Controller 层）。
add_library(weather_controller
    src/event.cpp
    src/weather.cpp
    src/weather_controller.cpp
)

target_include_directories(weather_controller
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 依赖：farm（自动传播 crop 的 include）、time_service（提供 mud::TimeService）
target_link_libraries(weather_controller
    PUBLIC
        farm
        time_service
)

# 源文件含 UTF-8 中文，MSVC 需显式 utf-8 否则按本机代码页(936)读取报错
if(MSVC)
    target_compile_options(weather_controller PRIVATE /utf-8)
endif()