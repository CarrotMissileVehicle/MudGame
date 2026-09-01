# MudGame 源码子目录聚合入口。
# 按模块分别引入各子库目标：对象模型、采矿控制器、时间服务、命令解析器。
add_subdirectory(objects)
add_subdirectory(controller/mining_controller)
add_subdirectory(models/timeService)
add_subdirectory(view/cmdParser)