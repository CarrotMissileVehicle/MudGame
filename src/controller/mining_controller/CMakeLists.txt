add_library(mining_controller
    src/mining_controller.cpp
    src/mining_calculator.cpp
    src/mining_state.cpp
    src/ore_data.cpp
    src/ore_layer.cpp
)

target_link_libraries(mining_controller
    PUBLIC
        objects
)

target_include_directories(mining_controller
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
