# weather_controller 模块测试：天气/事件系统冲突修复验证。
# 链接 weather_controller（PUBLIC 依赖 farm / time_service 随目标传递）。
function(mudgame_add_weather_test target)
    add_executable(${target} ${ARGN})
    target_link_libraries(${target} PRIVATE gtest_main weather_controller)
    target_compile_options(${target} PRIVATE -Wall -Wextra)
    add_test(NAME ${target} COMMAND ${target})
endfunction()

mudgame_add_weather_test(weather_event_test
    event_system_test.cpp)

# DEF-347：跨天推进补算未浇水天数（帧跳过多天的虫害判定）
# Farm/FarmLand/Crop 实现编入 Jerry 模块（可执行），测试目标需显式附加其实现。
mudgame_add_weather_test(weather_controller_test
    weather_controller_test.cpp
    ${CMAKE_SOURCE_DIR}/src/Controller/Farm/src/Farm.cpp
    ${CMAKE_SOURCE_DIR}/src/Controller/Farm/src/FarmLand.cpp
    ${CMAKE_SOURCE_DIR}/src/Controller/Crop/src/Crop.cpp
    ${CMAKE_SOURCE_DIR}/src/Controller/Food/src/Food.cpp)