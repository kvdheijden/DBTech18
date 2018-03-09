//
// Created by koenv on 09/03/2018.
//

#ifndef QUICKSILVER_CARDSTAT_H
#define QUICKSILVER_CARDSTAT_H

#include "stdafx.h"

struct cardStat {
    uint32_t noOut;
    uint32_t noPaths;
    uint32_t noIn;

    void print() const;
};


#endif //QUICKSILVER_CARDSTAT_H
