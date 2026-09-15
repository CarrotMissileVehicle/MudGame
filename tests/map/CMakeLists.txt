# map 模块测试：地图导航（H13 基类显式抛错 / 具体位置移动图）。
# map 库无外部依赖，测试直接链接。
function(mudgame_add_map_test target)
    add_executable(${target} ${ARGN})
    target_link_libraries(${target} PRIVATE gtest_main map)
    target_compile_options(${target} PRIVATE -Wall -Wextra)
    add_test(NAME ${target} COMMAND ${target})
endfunction()

mudgame_add_map_test(position_test
    position_test.cpp)
