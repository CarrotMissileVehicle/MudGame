#include "../include/Fish.h"

Fish::Fish(float probability, int fishExp, int satiationRecovery,
           int sellingPrice, int buyingPrice)
        : Food(satiationRecovery, sellingPrice, buyingPrice),
          probability(probability),
          fishExp(fishExp) {}

Fish::~Fish() {}

float Fish::getProbability() const { return probability; }

int Fish::getFishExp() const { return fishExp; }
