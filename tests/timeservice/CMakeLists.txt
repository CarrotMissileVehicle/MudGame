# timeservice 模块测试：按能力方向各建一个可执行目标。
# time_service 以 PUBLIC 暴露其 include 目录，测试目标链接 time_service
# 即可获得 GameDateTime 与 TimeService 接口；gtest_main 提供 main() 入口。
# 方向映射：
#   tick_engine → timeservice_tick_test          （TS-TE）
#   time_scale → timeservice_scale_test          （TS-SC）
#   precision → timeservice_precision_test       （TS-PR）
#   calendar_event → timeservice_calendar_test   （TS-CE）
#   timer_queue → timeservice_timer_test         （TS-TQ）
#   cancel_reset → timeservice_cancel_test       （TS-CR）
#   clock_anomaly → timeservice_clock_anomaly_test（TS-CA）
#   load_concurrency → timeservice_load_test     （TS-LC）
function(mudgame_add_timeservice_test target)
    add_executable(${target} ${ARGN})
    target_link_libraries(${target} PRIVATE gtest_main time_service)
    target_compile_options(${target} PRIVATE -Wall -Wextra)
    add_test(NAME ${target} COMMAND ${target})
endfunction()

mudgame_add_timeservice_test(timeservice_calendar_test
    calendar_event/calendar_test.cpp)

mudgame_add_timeservice_test(timeservice_tick_test
    tick_engine/tick_test.cpp)

mudgame_add_timeservice_test(timeservice_scale_test
    time_scale/scale_test.cpp
    time_scale/bound_test.cpp)

mudgame_add_timeservice_test(timeservice_timer_test
    timer_queue/timer_test.cpp
    timer_queue/schedule_validation_test.cpp)

mudgame_add_timeservice_test(timeservice_cancel_test
    cancel_reset/cancel_test.cpp)

mudgame_add_timeservice_test(timeservice_precision_test
    precision/precision_test.cpp)

mudgame_add_timeservice_test(timeservice_clock_anomaly_test
    clock_anomaly/clock_anomaly_test.cpp)

mudgame_add_timeservice_test(timeservice_load_test
    load_concurrency/load_test.cpp)