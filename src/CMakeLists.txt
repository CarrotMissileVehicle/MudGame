# MudGame 源码子目录聚合入口。
# 按模块分别引入各子库目标：对象模型、控制器、时间服务、视图等。
#
# 说明：部分源码使用仓库根相对形式的引号 include（如 "Controller/Game/include/Game.h"、
# "Model/Map/include/Home.h"），因此把 src 根加入全局 include 搜索路径，
# 使任意子模块编译时均可解析这类路径。

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# ---- 基础物品 / 模型 ----
add_subdirectory(Objects)                # objects（object，小写）
add_subdirectory(Model/Timeservice)      # time_service
add_subdirectory(Model/Objects)          # model_objects（Object 富接口）
add_subdirectory(Model/Bag)              # bag
add_subdirectory(Model/Map)              # map
add_subdirectory(Model/Playerstates)     # playerstates

# ---- 控制器 ----
add_subdirectory(Controller/Objects)     # controller_objects（Object 默认构造）
add_subdirectory(Controller/Crop)        # crop
add_subdirectory(Controller/Fertilizer)  # fertilizer
add_subdirectory(Controller/Farm)        # farm
add_subdirectory(Controller/Game)        # game
add_subdirectory(Controller/Mining_controller)  # mining_controller
add_subdirectory(Controller/Player)      # player

# ---- 工具 / 视图 ----
add_subdirectory(Tool)                   # player_serializer
add_subdirectory(View/Cmdparser)         # cmd_parser
add_subdirectory(View/Statusview)        # statusview

# ---- 占位模块（未实现依赖，暂不纳入构建）----
# add_subdirectory(Controller/Tool)      # 依赖 tool.h/tools.h/player.h/inventory.h
# add_subdirectory(Controller/Weather)   # 依赖 event.h/weather.h