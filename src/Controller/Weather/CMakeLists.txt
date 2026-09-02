# weather_controller 库：天气与事件控制器（Controller/Weather）。
#
# 注意：weather_Controller 依赖的 event.h / weather.h 等占位头文件尚未实现，
# 故该模块暂不纳入构建（src/CMakeLists.txt 中已注释）。
# 已将其中的已删除 TimeSystem 引用改为 mud::TimeService（Timeservice 模块）。
add_library(weather_controller
    src/weather_Controller.cpp
)

target_include_directories(weather_controller
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(weather_controller
    PUBLIC
        time_service
        farm
)