# cmdparser 模块测试：语法/语义/上下文/逻辑/交互 五层各一个可执行目标。
# cmd_parser 以 PUBLIC 链接 mining_controller / time_service / view_primitives / CLI11，
# 因此各测试目标只需链接 cmd_parser 即可传递获得其接口依赖
# （含 mining_controller 的 MUDGAME_DATA_DIR 数据目录宏）。
# gtest_main 提供 main() 入口；层级映射：
#   syntax → cmdparser_syntax_test   （CP-S）
#   semantic → cmdparser_semantic_test（CP-M）
#   context → cmdparser_context_test  （CP-C）
#   logic → cmdparser_logic_test      （CP-L）
#   interactive → cmdparser_interactive_test（交互式输入）
function(mudgame_add_cmdparser_test target)
    add_executable(${target} ${ARGN})
    target_link_libraries(${target} PRIVATE gtest_main cmd_parser)
    target_compile_options(${target} PRIVATE -Wall -Wextra)
    add_test(NAME ${target} COMMAND ${target})
endfunction()

mudgame_add_cmdparser_test(cmdparser_syntax_test
    syntax/input_parser_syntax_test.cpp)

mudgame_add_cmdparser_test(cmdparser_semantic_test
    semantic/input_parser_semantic_test.cpp
    semantic/parse_double_test.cpp)

mudgame_add_cmdparser_test(cmdparser_context_test
    context/state_gate_test.cpp)

mudgame_add_cmdparser_test(cmdparser_logic_test
    logic/connector_dispatch_test.cpp)

mudgame_add_cmdparser_test(cmdparser_interactive_test
    interactive/interactive_input_test.cpp)