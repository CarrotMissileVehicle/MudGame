# model_objects 库：富对象基类（Object，含 name/desc/health 等完整接口）。
# 供 Model/Bag 等使用。
add_library(model_objects
    src/Object.cpp
)

target_include_directories(model_objects
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)