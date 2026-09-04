//
// Created by z2996 on 2026/8/25.
//

#include "../include/Town.h"

#include "Model/Map/include/Home.h"

Position * Town::GoDown() {
    return new Home();
}
