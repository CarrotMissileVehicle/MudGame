# tool_controller 库：工具模型与控制器（mud::tool），自包含，无外部占位依赖。
add_library(tool_controller
    src/tool.cpp
    src/tool_Controller.cpp
)

target_include_directories(tool_controller
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 源文件含 UTF-8 中文，MSVC 需显式 utf-8 否则按本机代码页(936)读取报错
if(MSVC)
    target_compile_options(tool_controller PRIVATE /utf-8)
endif()