# controller_objects 库：物品基类对象（Object，大写，默认构造）。
# 供 Crop / Fertilizer 等派生使用。
add_library(controller_objects
    src/Object.cpp
)

target_include_directories(controller_objects
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)