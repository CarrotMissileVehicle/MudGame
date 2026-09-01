#include "../include/Food.h"

Food::Food(int satiationRecovery, int sellingPrice, int buyingPrice)
        : Object(sellingPrice, buyingPrice),
          satiationRecovery(satiationRecovery) {}

Food::~Food() {}

int Food::getSatiationRecovery() const { return satiationRecovery; }
