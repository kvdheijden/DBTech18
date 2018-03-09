//
// Created by koenv on 09/03/2018.
//

#include "cardStat.h"

void cardStat::print() const {
    std::cout << "(" << noOut << ", " << noPaths << ", " << noIn << ")" << std::endl;
}
