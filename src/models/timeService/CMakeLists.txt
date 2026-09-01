# time_service 库：游戏时间服务模块（TimeService）。
add_library(time_service
    src/time_service.cpp
)

# 对外暴露本模块头文件目录。
target_include_directories(time_service
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)