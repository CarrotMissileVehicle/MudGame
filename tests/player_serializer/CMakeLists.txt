# player_serializer 模块测试：存档/读档往返。
# 链接 player_serializer（PUBLIC 依赖 game / player / tool_controller 会传递）。
function(mudgame_add_serializer_test target)
    add_executable(${target} ${ARGN})
    target_link_libraries(${target} PRIVATE gtest_main player_serializer)
    target_compile_options(${target} PRIVATE -Wall -Wextra)
    add_test(NAME ${target} COMMAND ${target})
endfunction()

mudgame_add_serializer_test(player_serializer_roundtrip_test
    player_serializer_roundtrip_test.cpp)