# playerstates 模块测试：玩家状态模型。
add_executable(player_state_test player_state_test.cpp)
target_link_libraries(player_state_test PRIVATE gtest_main playerstates)
target_compile_options(player_state_test PRIVATE -Wall -Wextra)
add_test(NAME player_state_test COMMAND player_state_test)
