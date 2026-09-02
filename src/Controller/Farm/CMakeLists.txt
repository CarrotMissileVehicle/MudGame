# farm 库：农田体系（仅头文件）。依赖 crop。
add_library(farm INTERFACE)

target_include_directories(farm
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(farm
    INTERFACE
        crop
)