# fishing 模块测试（黑盒）：tickFish 命中概率、日夜 catchRate、空鱼池。
# Fishing/Fish 为 Jerry 模块（源码编进 MudGame 目标），测试直接编译所需源文件。
function(mudgame_add_fishing_test target)
    add_executable(${target} ${ARGN})
    target_link_libraries(${target} PRIVATE gtest_main model_objects)
    target_sources(${target} PRIVATE
        ${CMAKE_SOURCE_DIR}/src/Controller/Fishing/src/FishingController.cpp
        ${CMAKE_SOURCE_DIR}/src/Controller/Fish/src/Fish.cpp
        ${CMAKE_SOURCE_DIR}/src/Controller/Food/src/Food.cpp)
    target_include_directories(${target} PRIVATE
        ${CMAKE_SOURCE_DIR}/src/Controller/Fishing/include
        ${CMAKE_SOURCE_DIR}/src/Controller/Fish/include
        ${CMAKE_SOURCE_DIR}/src/Controller/Food/include
        ${CMAKE_SOURCE_DIR}/src/Model/Objects/include
        ${CMAKE_SOURCE_DIR}/src/Model/Timeservice/include)
    target_compile_options(${target} PRIVATE -Wall -Wextra)
    add_test(NAME ${target} COMMAND ${target})
endfunction()

mudgame_add_fishing_test(fishing_core_test
    core/fishing_core_test.cpp)
