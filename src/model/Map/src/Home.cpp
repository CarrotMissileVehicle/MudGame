//
// Created by z2996 on 2026/8/25.
//

#include "../include/Home.h"

#include "model/Map/include/Coast.h"
#include "model/Map/include/Farmland.h"
#include "model/Map/include/Mine.h"
#include "model/Map/include/Town.h"

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
