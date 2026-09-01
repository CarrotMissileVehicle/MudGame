cmake_minimum_required(VERSION 4.0)
project(MudGame)

set(CMAKE_CXX_STANDARD 20)

add_executable(MudGame main.cpp
        src/Controller/Objects/src/Object.cpp
        src/Controller/Food/src/Food.cpp
        src/Controller/Crop/src/Crop.cpp
        src/Controller/Crop/src/Cabbage.cpp
        src/Controller/Crop/src/Carrot.cpp
        src/Controller/Crop/src/Tomato.cpp
        src/Controller/Crop/src/Pumpkin.cpp
        src/Controller/Crop/src/Lingzhi.cpp
        src/Controller/Fertilizer/src/Fertilizer.cpp
        src/Controller/Fertilizer/src/NormalFertilizer.cpp
        src/Controller/Fertilizer/src/AdvancedFertilizer.cpp
        src/Controller/Farm/src/Farm.cpp
        src/Controller/Farm/src/FarmLand.cpp
        src/Controller/Fish/src/Fish.cpp
        src/Controller/Fish/src/Crucian.cpp
        src/Controller/Fish/src/GrassCarp.cpp
        src/Controller/Fish/src/Perch.cpp
        src/Controller/Fish/src/RainbowTrout.cpp
        src/Controller/Fish/src/KingCrab.cpp
        src/Controller/Farming/src/FarmingController.cpp
        src/Controller/Fishing/src/FishingController.cpp
        src/Controller/Market/src/Market.cpp
        src/Controller/Market/src/Shop.cpp
        src/Controller/Market/src/ShopItem.cpp
)

target_include_directories(MudGame PRIVATE
        src/Controller/Objects/include
        src/Controller/Food/include
        src/Controller/Crop/include
        src/Controller/Fertilizer/include
        src/Controller/Farm/include
        src/Controller/Fish/include
        src/Controller/Market/include
)
