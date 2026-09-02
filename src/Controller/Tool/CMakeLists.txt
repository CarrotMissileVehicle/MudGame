# tool_controller 库：工具控制器（Controller/Tool）。
#
# 注意：tool_Controller 依赖的 tool.h / tools.h / player.h / inventory.h
# 均为尚未实现的占位头文件，故该模块暂不纳入构建（src/CMakeLists.txt 中已注释）。
# 待占位依赖补齐后可恢复 add_subdirectory。
add_library(tool_controller
    src/tool_Controller.cpp
)

target_include_directories(tool_controller
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)