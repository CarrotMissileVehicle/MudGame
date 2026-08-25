add_library(objects
    src/object.cpp
)

target_include_directories(objects
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
