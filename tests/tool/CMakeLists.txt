# tool 模块测试：工具耐久、修复费用与读档还原。
# 链接 tool_controller（自包含，无外部依赖）。
function(mudgame_add_tool_test target)
    add_executable(${target} ${ARGN})
    target_link_libraries(${target} PRIVATE gtest_main tool_controller)
    target_compile_options(${target} PRIVATE -Wall -Wextra)
    add_test(NAME ${target} COMMAND ${target})
endfunction()

mudgame_add_tool_test(tool_repair_test
    tool_repair_test.cpp)