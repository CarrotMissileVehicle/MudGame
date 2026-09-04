# MudGame 源码子目录聚合入口。
# 按模块分别引入各子库目标：采矿控制器、时间服务、命令解析器。
# objects 基类库直接定义在顶层 CMakeLists.txt 中。
add_subdirectory(controller/mining_controller)
add_subdirectory(models/timeService)
add_subdirectory(view/cmdParser)