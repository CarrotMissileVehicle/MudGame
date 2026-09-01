add_library(cmd_parser
    src/input_parser.cpp
    src/connector.cpp
)

# connector.h 的 HandlerContext 依赖 TimeService 与 MiningState，
# 通过链接 time_service / mining_controller 继承其 PUBLIC include 目录。
target_link_libraries(cmd_parser
    PUBLIC
        mining_controller
        time_service
        CLI11::CLI11
)

target_include_directories(cmd_parser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)