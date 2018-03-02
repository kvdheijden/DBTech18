//
// Created by Nikolay Yakovets on 2018-02-01.
//

#ifndef QS_SIMPLEESTIMATOR_H
#define QS_SIMPLEESTIMATOR_H

#include "Estimator.h"
#include "SimpleGraph.h"
#include <set>

class SimpleEstimator : public Estimator {

    std::shared_ptr<SimpleGraph> graph;

    //first attempt:
    // Different counts needed after prep.
    // Total number of nodes with a specific outgoing label.
    std::vector<uint32_t> nodesWithOutLabel;
    // Total number of nodes with a specific incoming label.
    std::vector<uint32_t> nodesWithInLabel;
    // The ~probability for each label to occur (same for incoming and outgoing).
    std::vector<float> labelProbs;
    // Number of nodes and number of labels.
    uint32_t N, L;


    //Brute force:
    std::vector<std::vector<std::vector<std::pair<uint32_t,uint32_t>>>> summary;
    uint32_t a_start_vertices;
    std::set<uint32_t> unique_end_vertices;


public:
    explicit SimpleEstimator(std::shared_ptr<SimpleGraph> &g);
    ~SimpleEstimator() = default;

    void prepare() override ;
    cardStat estimate(RPQTree *q) override ;

    //first attempt:
    void prepareFirst();
    cardStat estimateFirst(RPQTree *q);
    float calcProb(RPQTree *q);

    //brute force:
    void prepareBruteForce();
    cardStat estimateBruteForce(RPQTree *q);
    std::string treeToString(RPQTree *q);
    int subestimateBruteForce(std::vector<std::string> path, int node, bool calculate_start_vertices);
    int estimateBruteForceStart(RPQTree *q);
    int estimateBruteForceEnd(RPQTree *q);
};


#endif //QS_SIMPLEESTIMATOR_H
