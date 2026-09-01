#include "../include/Fish.h"

Fish::Fish(float probability, int fishExp, int sellingPrice, int buyingPrice)
        : Object(sellingPrice, buyingPrice),
          probability(probability),
          fishExp(fishExp) {}

Fish::~Fish() {}

float Fish::getProbability() const { return probability; }

int Fish::getFishExp() const { return fishExp; }
