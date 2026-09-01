//
// Created by z2996 on 2026/8/25.
//

#include "../include/Mine.h"

#include "model/Map/include/Home.h"

Position * Mine::GoUp() {
    return new Home();
}
