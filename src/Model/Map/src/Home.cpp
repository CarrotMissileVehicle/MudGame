//
// Created by z2996 on 2026/8/25.
//

#include "../include/Home.h"

#include "Model/Map/include/Coast.h"
#include "Model/Map/include/Farmland.h"
#include "Model/Map/include/Mine.h"
#include "Model/Map/include/Town.h"

Position * Home::GoUp() {
    return new Town();
}

Position * Home::GoDown() {
    return new Mine();
}

Position * Home::GoLeft() {
    return new Farmland();
}

Position * Home::GoRight() {
    return new Coast();
}
