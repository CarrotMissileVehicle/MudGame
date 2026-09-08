# food 库：食物基类（仅头文件）。依赖物品基类 model_objects。
add_library(food INTERFACE)

target_include_directories(food
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(food
    INTERFACE
        model_objects
)