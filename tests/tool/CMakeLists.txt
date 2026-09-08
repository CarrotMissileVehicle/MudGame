# tool 模块测试：ToolController 按 ToolId 查找（DEF-006 回归）。
# tool_controller 自包含（无外部依赖），测试目标只需链接 tool_controller。
function(mudgame_add_tool_test target)
    add_executable(${target} ${ARGN})
    target_link_libraries(${target} PRIVATE gtest_main tool_controller)
    target_compile_options(${target} PRIVATE -Wall -Wextra)
    add_test(NAME ${target} COMMAND ${target})
endfunction()

mudgame_add_tool_test(tool_lookup_test
    lookup/tool_lookup_test.cpp)
