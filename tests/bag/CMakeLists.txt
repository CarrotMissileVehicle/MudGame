# bag 模块测试：拷贝语义（DEF-002 回归）与堆叠功能两个可执行目标。
# bag 以 PUBLIC 链接 model_objects，测试目标只需链接 bag 即可。
function(mudgame_add_bag_test target)
    add_executable(${target} ${ARGN})
    target_link_libraries(${target} PRIVATE gtest_main bag)
    target_compile_options(${target} PRIVATE -Wall -Wextra)
    add_test(NAME ${target} COMMAND ${target})
endfunction()

mudgame_add_bag_test(bag_copy_test
    copy_semantics/bag_copy_test.cpp)

mudgame_add_bag_test(bag_stack_test
    stacking/bag_stack_test.cpp)
