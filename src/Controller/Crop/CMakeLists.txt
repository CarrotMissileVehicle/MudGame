# crop 库：农作物体系（仅头文件）。依赖物品基类 controller_objects。
add_library(crop INTERFACE)

target_include_directories(crop
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(crop
    INTERFACE
        controller_objects
)