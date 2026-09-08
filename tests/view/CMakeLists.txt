# view 模块测试：面板渲染（用 StringRenderer 断言输出）。
# 链接 view_panels：其 PUBLIC 依赖 view_primitives / map / playerstates 会随之传递。
function(mudgame_add_view_test target)
    add_executable(${target} ${ARGN})
    target_link_libraries(${target} PRIVATE gtest_main view_panels)
    target_compile_options(${target} PRIVATE -Wall -Wextra)
    add_test(NAME ${target} COMMAND ${target})
endfunction()

mudgame_add_view_test(view_panel_test
    panel_render_test.cpp)