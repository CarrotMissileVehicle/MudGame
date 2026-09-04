# mining_controller 模块测试：按测试方向各建一个可执行目标。
# mining_controller 以 PUBLIC 暴露 include 目录并注入 MUDGAME_DATA_DIR，
# 测试目标链接 mining_controller 即可获得接口与数据路径；gtest_main 提供 main()。
# 方向映射：
#   precondition → mining_precondition_test      （MIN-PR）
#   core_flow → mining_core_flow_test            （MIN-CF）
#   probability → mining_probability_test        （MIN-PB）
#   economy_bag → mining_economy_bag_test        （MIN-EC）
#   transaction → mining_transaction_test        （MIN-TX）
#   resource_refresh → mining_refresh_test       （MIN-RF）
#   concurrency → mining_concurrency_test        （MIN-CC）
#   boundary → mining_boundary_test              （MIN-BD）
function(mudgame_add_mining_test target)
    add_executable(${target} ${ARGN})
    target_link_libraries(${target} PRIVATE gtest_main mining_controller)
    target_compile_options(${target} PRIVATE -Wall -Wextra)
    add_test(NAME ${target} COMMAND ${target})
endfunction()

mudgame_add_mining_test(mining_core_flow_test
    core_flow/core_flow_test.cpp)

mudgame_add_mining_test(mining_precondition_test
    precondition/precondition_test.cpp)

mudgame_add_mining_test(mining_probability_test
    probability/probability_test.cpp)

mudgame_add_mining_test(mining_boundary_test
    boundary/boundary_test.cpp)

mudgame_add_mining_test(mining_concurrency_test
    concurrency/concurrency_test.cpp)

mudgame_add_mining_test(mining_transaction_test
    transaction/transaction_test.cpp)

mudgame_add_mining_test(mining_economy_bag_test
    economy_bag/economy_bag_test.cpp)

mudgame_add_mining_test(mining_refresh_test
    resource_refresh/resource_refresh_test.cpp)

# 事件/工具接入采矿：额外链接 tool_controller 与 weather_controller。
add_executable(mining_event_tool_test
    event_tool/event_tool_test.cpp)
target_link_libraries(mining_event_tool_test
    PRIVATE gtest_main mining_controller tool_controller weather_controller)
target_compile_options(mining_event_tool_test PRIVATE -Wall -Wextra)
add_test(NAME mining_event_tool_test COMMAND mining_event_tool_test)