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

    // Sampling factor = 1/sampling_factor, so 2 = 50%, 4 = 25%, etc.
    const uint32_t sampling_factor = 128;

    const bool calculate_in_and_out = false;
public:
    explicit SimpleEstimator(std::shared_ptr<SimpleGraph> &g);
    ~SimpleEstimator() = default;

    void prepare() override ;
    cardStat estimate(RPQTree *q) override ;

    // Change this to 0 for path prob. , 1 for brute force and 2 for sampling.
    int estimateMethod = 2;

    //first attempt:
    void prepareFirst();
    cardStat estimateFirst(RPQTree *q);
    void convertQuery(RPQTree *q, std::vector<uint32_t> &query);
    float calcProb(std::vector<uint32_t> query);
//    void countPaths(std::vector<uint32_t> &pathMatrix, uint32_t node);
//    void countPaths(std::vector<std::pair<std::vector<uint32_t>, uint32_t>> &pathMatrix, uint32_t node);
//    void countPaths(std::vector<std::pair<std::vector<std::pair<std::vector<uint32_t>, uint32_t>>, uint32_t>> &pathMatrix,
//                    uint32_t node);
    template <typename T> void countPaths(std::vector<T> &path, uint32_t node);

    //brute force:
    void prepareBruteForce();
    cardStat estimateBruteForce(RPQTree *q);
    //sampling:
    void prepareSampling();
    cardStat estimateSampling(RPQTree *q);
    std::string treeToString(RPQTree *q);
    int subestimateBruteForce(std::vector<std::string> path, uint32_t node, bool calculate_start_vertices);
};


#endif //QS_SIMPLEESTIMATOR_H
