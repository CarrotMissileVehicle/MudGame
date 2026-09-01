add_library(tool_controller
    src/tool.cpp
    src/tool_controller.cpp
    ${CMAKE_SOURCE_DIR}/src/model/Objects/src/Object.cpp
)

# 依赖：玩家/背包桩（留桩）；物品基类 Object
target_include_directories(tool_controller
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src/models/player/include
        ${CMAKE_SOURCE_DIR}/src/models/inventory/include
        ${CMAKE_SOURCE_DIR}/src/model/Objects/include
)

# 源文件含 UTF-8 中文，MSVC 需显式 utf-8 否则按本机代码页(936)读取报错
if(MSVC)
    target_compile_options(tool_controller PRIVATE /utf-8)
endif()