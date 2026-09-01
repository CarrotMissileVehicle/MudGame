# 注：Object 基类已迁至 src/model/Objects（工具模块内直接编译其源文件），
# 原 src/objects 目录已删除，故不再 add_subdirectory(objects)。
add_subdirectory(controller/mining_controller)
add_subdirectory(controller/weather)
add_subdirectory(controller/tool)
add_subdirectory(models/timeService)
add_subdirectory(view/cmdParser)