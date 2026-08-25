add_library(mining_controller
    src/mining_controller.cpp
)

target_link_libraries(mining_controller
    PUBLIC
        objects
)

target_include_directories(mining_controller
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)