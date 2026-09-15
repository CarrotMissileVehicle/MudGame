# model_objects 模块测试：Object 数值校验。
add_executable(object_validation_test object_validation_test.cpp)
target_link_libraries(object_validation_test PRIVATE gtest_main model_objects)
target_compile_options(object_validation_test PRIVATE -Wall -Wextra)
add_test(NAME object_validation_test COMMAND object_validation_test)
