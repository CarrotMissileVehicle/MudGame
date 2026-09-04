# map 库：地图/位置模型。独立，无外部依赖。
add_library(map
    src/Coast.cpp
    src/Farmland.cpp
    src/Home.cpp
    src/Mine.cpp
    src/Position.cpp
    src/Town.cpp
)

target_include_directories(map
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)