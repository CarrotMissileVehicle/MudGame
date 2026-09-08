# food 库：食物基类（Crop / Fish 的公共父类）。依赖物品基类 model_objects。
add_library(food
    src/Food.cpp
)

target_include_directories(food
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(food
    PUBLIC
        model_objects
)
