# cmd_parser 库：命令行解析与命令派发模块（InputParser / Connector / ParameterCollector）。
add_library(cmd_parser
    src/input_parser.cpp
    src/connector.cpp
    src/parameter_collector.cpp
)

# connector.h 的 HandlerContext 依赖 TimeService 与 MiningHandler，
# 通过链接 time_service / mining_controller 继承其 PUBLIC include 目录。
# CLI11::CLI11 提供命令行解析能力。
# ParameterCollector 通过 Renderer 输出提示。
target_link_libraries(cmd_parser
    PUBLIC
        mining_controller
        time_service
        view_primitives
        CLI11::CLI11
)

# 对外暴露本模块头文件目录。
target_include_directories(cmd_parser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)