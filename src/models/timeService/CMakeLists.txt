add_library(time_service
    src/time_service.cpp
)

target_include_directories(time_service
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)