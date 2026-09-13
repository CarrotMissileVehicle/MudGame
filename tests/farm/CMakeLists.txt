# farm 模块测试：施肥语义（DEF-008 回归）与浇水/生长联动。
# farm 为 INTERFACE 库（Crop 体系为头文件实现，源码编进 MudGame 目标），
# 测试目标直接编译所需 Jerry 模块源文件以获得符号。
function(mudgame_add_farm_test target)
    add_executable(${target} ${ARGN})
    target_link_libraries(${target} PRIVATE gtest_main model_objects)
    target_sources(${target} PRIVATE
        ${CMAKE_SOURCE_DIR}/src/Controller/Food/src/Food.cpp
        ${CMAKE_SOURCE_DIR}/src/Controller/Crop/src/Crop.cpp
        ${CMAKE_SOURCE_DIR}/src/Controller/Crop/src/Cabbage.cpp
        ${CMAKE_SOURCE_DIR}/src/Controller/Farm/src/FarmLand.cpp
    )
    target_include_directories(${target} PRIVATE
        ${CMAKE_SOURCE_DIR}/src/Controller/Food/include
        ${CMAKE_SOURCE_DIR}/src/Controller/Crop/include
        ${CMAKE_SOURCE_DIR}/src/Controller/Farm/include
    )
    target_compile_options(${target} PRIVATE -Wall -Wextra)
    add_test(NAME ${target} COMMAND ${target})
endfunction()

mudgame_add_farm_test(farm_fertilize_test
    fertilize/farm_fertilize_test.cpp)

mudgame_add_farm_test(farm_grow_test
    water_grow/farm_grow_test.cpp)

mudgame_add_farm_test(farming_bounds_test
    bounds/farming_bounds_test.cpp
    ${CMAKE_SOURCE_DIR}/src/Controller/Farm/src/Farm.cpp
    ${CMAKE_SOURCE_DIR}/src/Controller/Farming/src/FarmingController.cpp)
target_sources(farming_bounds_test PRIVATE
    ${CMAKE_SOURCE_DIR}/src/Controller/Fertilizer/src/Fertilizer.cpp
    ${CMAKE_SOURCE_DIR}/src/Controller/Fertilizer/src/NormalFertilizer.cpp
    ${CMAKE_SOURCE_DIR}/src/Controller/Fertilizer/src/AdvancedFertilizer.cpp)
target_include_directories(farming_bounds_test PRIVATE
    ${CMAKE_SOURCE_DIR}/src/Controller/Fertilizer/include
    ${CMAKE_SOURCE_DIR}/src/Controller/Farming/include
    ${CMAKE_SOURCE_DIR}/src/Model/Timeservice/include)
