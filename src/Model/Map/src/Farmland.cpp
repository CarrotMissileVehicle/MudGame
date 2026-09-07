//
// Created by z2996 on 2026/8/25.
//

#include "../include/Farmland.h"

#include "Model/Map/include/Home.h"

Position * Farmland::GoRight() {
    return new Home();
}
