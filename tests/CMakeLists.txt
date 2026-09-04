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

# ---- time_service 模块：按测试方向各建一个可执行目标 ----
# time_service 以 PUBLIC 暴露其 include 目录，测试目标链接 time_service 即可获得
# GameDateTime 与 TimeService 接口；gtest_main 提供 main() 入口。
function(mudgame_add_timeservice_test target)
    add_executable(${target} ${ARGN})
    target_link_libraries(${target} PRIVATE gtest_main time_service)
    target_compile_options(${target} PRIVATE -Wall -Wextra)
    add_test(NAME ${target} COMMAND ${target})
endfunction()

mudgame_add_timeservice_test(timeservice_calendar_test
    timeservice/calendar_event/calendar_test.cpp)

mudgame_add_timeservice_test(timeservice_tick_test
    timeservice/tick_engine/tick_test.cpp)

mudgame_add_timeservice_test(timeservice_scale_test
    timeservice/time_scale/scale_test.cpp)

mudgame_add_timeservice_test(timeservice_timer_test
    timeservice/timer_queue/timer_test.cpp)

mudgame_add_timeservice_test(timeservice_cancel_test
    timeservice/cancel_reset/cancel_test.cpp)

mudgame_add_timeservice_test(timeservice_precision_test
    timeservice/precision/precision_test.cpp)

mudgame_add_timeservice_test(timeservice_clock_anomaly_test
    timeservice/clock_anomaly/clock_anomaly_test.cpp)

mudgame_add_timeservice_test(timeservice_load_test
    timeservice/load_concurrency/load_test.cpp)

# ---- mining_controller 模块：按测试方向各建一个可执行目标 ----
# mining_controller 以 PUBLIC 暴露 include 目录并注入 MUDGAME_DATA_DIR，
# 测试目标链接 time_service + mining_controller 即可获得接口与数据路径；
# gtest_main 提供 main() 入口。
function(mudgame_add_mining_test target)
    add_executable(${target} ${ARGN})
    target_link_libraries(${target} PRIVATE gtest_main mining_controller)
    target_compile_options(${target} PRIVATE -Wall -Wextra)
    add_test(NAME ${target} COMMAND ${target})
endfunction()

mudgame_add_mining_test(mining_core_flow_test
    mining_controller/core_flow/core_flow_test.cpp)

mudgame_add_mining_test(mining_precondition_test
    mining_controller/precondition/precondition_test.cpp)

mudgame_add_mining_test(mining_probability_test
    mining_controller/probability/probability_test.cpp)

mudgame_add_mining_test(mining_boundary_test
    mining_controller/boundary/boundary_test.cpp)

mudgame_add_mining_test(mining_concurrency_test
    mining_controller/concurrency/concurrency_test.cpp)

mudgame_add_mining_test(mining_transaction_test
    mining_controller/transaction/transaction_test.cpp)

mudgame_add_mining_test(mining_economy_bag_test
    mining_controller/economy_bag/economy_bag_test.cpp)

mudgame_add_mining_test(mining_refresh_test
    mining_controller/resource_refresh/resource_refresh_test.cpp)