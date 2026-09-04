# 单元测试聚合：GoogleTest + CTest。
#
# cmd_parser 以 PUBLIC 链接 mining_controller / time_service / CLI11，
# 因此各测试目标只需链接 cmd_parser 即可传递获得其接口依赖
# （含 mining_controller 的 MUDGAME_DATA_DIR 数据目录宏）。
# gtest_main 提供 main() 入口；测试沿用方案四层划分：
#   syntax → input_parser_syntax_test    （CP-S）
#   semantic → input_parser_semantic_test（CP-M）
#   context → state_gate_test            （CP-C）
#   logic → connector_dispatch_test      （CP-L）
enable_testing()

include(FetchContent)
FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(googletest)

# 生成一个 cmdparser 模块测试目标并注册到 CTest。
function(mudgame_add_cmdparser_test target)
    add_executable(${target} ${ARGN})
    target_link_libraries(${target} PRIVATE gtest_main cmd_parser)
    target_compile_options(${target} PRIVATE -Wall -Wextra)
    add_test(NAME ${target} COMMAND ${target})
endfunction()

mudgame_add_cmdparser_test(cmdparser_syntax_test
    cmdparser/syntax/input_parser_syntax_test.cpp)

mudgame_add_cmdparser_test(cmdparser_semantic_test
    cmdparser/semantic/input_parser_semantic_test.cpp)

mudgame_add_cmdparser_test(cmdparser_context_test
    cmdparser/context/state_gate_test.cpp)

mudgame_add_cmdparser_test(cmdparser_logic_test
    cmdparser/logic/connector_dispatch_test.cpp)