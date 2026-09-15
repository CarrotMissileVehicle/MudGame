# 单元测试聚合入口：与 src 的父子级结构保持一致。
# src 采用「每模块目录一个 CMakeLists.txt、父目录 add_subdirectory 聚合」；
# tests 同样处理：
#   - 本文件：共享的 CTest / googletest 装配 + 按模块聚合三个子目录。
#   - cmdparser/、timeservice/、mining_controller/ 各自的 CMakeLists.txt
#     定义本模块的 GoogleTest 可执行目标（见对应模块 CMakeLists 顶部映射）。
enable_testing()

include(FetchContent)
FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(googletest)

# ---- 按模块聚合各测试目标 ----
add_subdirectory(bag)
add_subdirectory(objects)
add_subdirectory(playerstates)
add_subdirectory(cmdparser)
add_subdirectory(farm)
add_subdirectory(fishing)
add_subdirectory(map)
add_subdirectory(market)
add_subdirectory(timeservice)
add_subdirectory(mining_controller)
add_subdirectory(tool)
add_subdirectory(player_serializer)
add_subdirectory(view)
add_subdirectory(weather)