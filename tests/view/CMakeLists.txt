# view 模块测试：Renderer / 渲染原语 / 面板 DTO 组装（S0 头文件补齐回归）。
# view_panels 以 PUBLIC 链接 view_primitives / map / playerstates，
# 传递获得 Renderer.h、dto.h 与枚举依赖。
function(mudgame_add_view_test target)
    add_executable(${target} ${ARGN})
    target_link_libraries(${target} PRIVATE gtest_main view_panels)
    target_compile_options(${target} PRIVATE -Wall -Wextra)
    add_test(NAME ${target} COMMAND ${target})
endfunction()

mudgame_add_view_test(view_render_test
    render/view_render_test.cpp)
