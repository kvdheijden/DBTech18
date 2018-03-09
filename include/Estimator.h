//
// Created by Nikolay Yakovets on 2018-01-31.
//

#ifndef QS_ESTIMATOR_H
#define QS_ESTIMATOR_H

#include "stdafx.h"

#include "cardStat.h"
#include "RPQTree.h"

class Estimator {

public:

    virtual void prepare() = 0;
    virtual cardStat estimate(RPQTree *q) = 0;

};


#endif //QS_ESTIMATOR_H
