# bag 库：背包模型。依赖富对象基类 model_objects。
add_library(bag
    src/Bag.cpp
)

target_include_directories(bag
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(bag
    PUBLIC
        model_objects
)