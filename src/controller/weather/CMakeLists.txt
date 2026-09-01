add_library(weather_controller
    src/weather.cpp
    src/event.cpp
    src/weather_controller.cpp
)

# 依赖：农田桩（留桩）
target_include_directories(weather_controller
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src/models/farm/include
)

# 源文件含 UTF-8 中文，MSVC 需显式 utf-8 否则按本机代码页(936)读取报错
if(MSVC)
    target_compile_options(weather_controller PRIVATE /utf-8)
endif()