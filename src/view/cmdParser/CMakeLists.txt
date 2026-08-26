add_library(cmd_parser
    src/input_parser.cpp
    src/connector.cpp
)
target_include_directories(cmd_parser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)