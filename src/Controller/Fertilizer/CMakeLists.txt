# fertilizer 库：肥料体系（仅头文件）。依赖物品基类 controller_objects。
add_library(fertilizer INTERFACE)

target_include_directories(fertilizer
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(fertilizer
    INTERFACE
        controller_objects
)