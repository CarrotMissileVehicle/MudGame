#include "../include/Fertilizer.h"

Fertilizer::Fertilizer(int speedUp, int sellingPrice, int buyingPrice)
        : Object(sellingPrice, buyingPrice),
          speedUp(speedUp) {}

Fertilizer::~Fertilizer() {}

int Fertilizer::getSpeedUp() const { return speedUp; }
