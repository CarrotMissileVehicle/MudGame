# weather 模块测试：配置查表（DEF-006 回归）与随机生成分布鲁棒性。
# weather_controller 以 PUBLIC 链接 farm / time_service，传递获得依赖。
function(mudgame_add_weather_test target)
    add_executable(${target} ${ARGN})
    target_link_libraries(${target} PRIVATE gtest_main weather_controller)
    target_compile_options(${target} PRIVATE -Wall -Wextra)
    add_test(NAME ${target} COMMAND ${target})
endfunction()

mudgame_add_weather_test(weather_config_test
    config_lookup/weather_config_test.cpp)
