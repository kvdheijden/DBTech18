//
// Created by Nikolay Yakovets on 2018-02-01.
//

#ifndef QS_SIMPLEESTIMATOR_H
#define QS_SIMPLEESTIMATOR_H

#include "Estimator.h"
#include "SimpleGraph.h"

class SimpleEstimator : public Estimator {

    std::shared_ptr<SimpleGraph> graph;
    // Different counts needed after prep.
    // Total number of nodes with a specific outgoing label.
    std::vector<uint32_t> nodesWithOutLabel;
    // Total number of nodes with a specific incoming label.
    std::vector<uint32_t> nodesWithInLabel;
    // The ~probability for each label to occur (same for incoming and outgoing).
    std::vector<float> labelProbs;
    // Number of nodes and number of labels.
    uint32_t N, L;

public:
    explicit SimpleEstimator(std::shared_ptr<SimpleGraph> &g);
    ~SimpleEstimator() = default;

    void prepare() override ;
    cardStat estimate(RPQTree *q) override ;
    float calcProb(RPQTree *q);

};


#endif //QS_SIMPLEESTIMATOR_H
