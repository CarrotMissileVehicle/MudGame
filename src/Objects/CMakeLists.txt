# objects 库：游戏物品基类模块（object）。
add_library(objects
    src/object.cpp
)

# 对外暴露本模块头文件目录。
target_include_directories(objects
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
