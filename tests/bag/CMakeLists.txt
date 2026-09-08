# bag 模块测试：背包堆叠逻辑。
# 链接 bag（PUBLIC 依赖 model_objects，头文件随目标传递）。
function(mudgame_add_bag_test target)
    add_executable(${target} ${ARGN})
    target_link_libraries(${target} PRIVATE gtest_main bag)
    target_compile_options(${target} PRIVATE -Wall -Wextra)
    add_test(NAME ${target} COMMAND ${target})
endfunction()

mudgame_add_bag_test(bag_stack_test
    bag_stack_test.cpp)