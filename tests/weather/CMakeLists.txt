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