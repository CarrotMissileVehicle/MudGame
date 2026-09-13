# market 模块测试（黑盒+灰盒）：浮动去重（DEF-107）、买卖语义、价格边界。
# Market 体系为 Jerry 模块（源码编进 MudGame 目标），测试直接编译所需源文件。
function(mudgame_add_market_test target)
    add_executable(${target} ${ARGN})
    target_link_libraries(${target} PRIVATE gtest_main model_objects)
    target_sources(${target} PRIVATE
        ${CMAKE_SOURCE_DIR}/src/Controller/Market/src/Market.cpp
        ${CMAKE_SOURCE_DIR}/src/Controller/Market/src/Shop.cpp
        ${CMAKE_SOURCE_DIR}/src/Controller/Market/src/ShopItem.cpp)
    target_include_directories(${target} PRIVATE
        ${CMAKE_SOURCE_DIR}/src/Controller/Market/include
        ${CMAKE_SOURCE_DIR}/src/Model/Objects/include
        ${CMAKE_SOURCE_DIR}/src/Model/Timeservice/include)
    target_compile_options(${target} PRIVATE -Wall -Wextra)
    add_test(NAME ${target} COMMAND ${target})
endfunction()

mudgame_add_market_test(market_fluctuation_test
    fluctuation/market_fluctuation_test.cpp)

mudgame_add_market_test(market_trade_test
    trade/market_trade_test.cpp)

mudgame_add_market_test(market_perf_test
    perf/market_perf_test.cpp)
