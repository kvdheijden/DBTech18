//
// Created by Nikolay Yakovets on 2018-02-01.
//

#ifndef QS_SIMPLEESTIMATOR_H
#define QS_SIMPLEESTIMATOR_H

#include "stdafx.h"

#include "Estimator.h"
#include "SimpleGraph.h"


class SimpleEstimator : public Estimator {

    std::shared_ptr<SimpleGraph> graph;

    // Different counts needed after prep.
    // Total number of nodes with a specific outgoing label.
    std::vector<uint32_t> nodesWithOutLabel;

    // Total number of nodes with a specific incoming label.
    std::vector<uint32_t> nodesWithInLabel;

    // Number of nodes and number of labels.
    const uint32_t N, L;

    // Number of dimensions in pathProb.
    const uint32_t D = 3;

    // The ~probability for each label to occur (same for incoming and outgoing).
    std::vector<float> labelProbs;

    // Probabilities of all paths, up to three consecutive.
    std::vector<std::pair<std::vector<std::pair<std::vector<float>, float>>, float>> pathProbs3;

    //Brute force:
    std::vector<std::vector<std::vector<std::pair<uint32_t,uint32_t>>>> summary;
    uint32_t a_start_vertices;
    std::set<uint32_t> unique_end_vertices;
    std::set<uint32_t> unique_end_vertices_per_vertex;
    std::set<uint32_t> sample;

    // RNG initialization
    std::random_device r;

    // Sampling factor = 1/sampling_factor, so 2 = 50%, 4 = 25%, etc.
    static const uint32_t sampling_factor = 128;

    // Calculate in and output nodes
    static const bool calculate_in_and_out = false;

    // Used algorithm
    static const enum {
        PATH_PROBABILITY,
        SAMPLING,
        BRUTE_FORCE
    } estimateMethod = SAMPLING;
public:
    explicit SimpleEstimator(std::shared_ptr<SimpleGraph> &g);

    ~SimpleEstimator() = default;

    void prepare() override ;
    cardStat estimate(RPQTree *q) override ;

    //first attempt:
    void prepareProbability();
    cardStat estimateProbability(RPQTree *q);
    void convertQuery(RPQTree *q, std::vector<uint32_t> &query);
    float calcProb(std::vector<uint32_t> query);
    template <typename T> void countPaths(std::vector<T> &path, uint32_t node);

    //brute force:
    void prepareBruteForce();
    cardStat estimateBruteForce(RPQTree *q);
    int subEstimateBruteForce(const std::vector<std::string>& path, uint32_t node, bool calculate_start_vertices);

    //sampling:
    void prepareSampling();
    cardStat estimateSampling(RPQTree *q);
};


#endif //QS_SIMPLEESTIMATOR_H
