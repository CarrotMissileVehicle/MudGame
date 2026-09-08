# player_serializer 模块测试：存读档往返保真（DEF-002/DEF-007 回归）。
# player_serializer 以 PUBLIC 链接 game / player / tool_controller，传递获得依赖。
function(mudgame_add_serializer_test target)
    add_executable(${target} ${ARGN})
    target_link_libraries(${target} PRIVATE gtest_main player_serializer)
    target_compile_options(${target} PRIVATE -Wall -Wextra)
    add_test(NAME ${target} COMMAND ${target})
endfunction()

mudgame_add_serializer_test(serializer_roundtrip_test
    roundtrip/serializer_roundtrip_test.cpp)
