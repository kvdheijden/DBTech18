//
// Created by Nikolay Yakovets on 2018-02-01.
//

#ifndef QS_SIMPLEESTIMATOR_H
#define QS_SIMPLEESTIMATOR_H

#define PATH_PROBABILITY    (0x01)
#define SAMPLING            (0x02)
#define BRUTE_FORCE         (0x03)
#define ESTIMATE_METHOD     SAMPLING

#if ESTIMATE_METHOD == PATH_PROBABILITY
#include <vector>
#elif (ESTIMATE_METHOD == SAMPLING) || (ESTIMATE_METHOD == BRUTE_FORCE)
#include <vector>
#include <set>
#include <random>
#endif

#include "Estimator.h"
#include "SimpleGraph.h"

class SimpleEstimator : public Estimator {
private:
    std::shared_ptr<SimpleGraph> graph;

#if ESTIMATE_METHOD == PATH_PROBABILITY
    // Number of dimensions in pathProb.
    static const uint32_t D = 3;

    // Path probability:
    std::vector<uint32_t> nodesWithOutLabel;
    std::vector<uint32_t> nodesWithInLabel;
    std::vector<float> labelProbabilities;
    std::vector<std::pair<std::vector<std::pair<std::vector<float>, float>>, float>> pathProbabilities;
#elif (ESTIMATE_METHOD == SAMPLING) || (ESTIMATE_METHOD == BRUTE_FORCE)
    // Sampling/Brute force:
    std::vector<std::vector<std::pair<uint32_t, uint32_t>>> summary;
    std::vector<std::vector<std::pair<uint32_t, uint32_t>>> r_summary;
    uint32_t n_start_vertices;
    std::set<uint32_t> unique_end_vertices;
    std::set<uint32_t> unique_end_vertices_per_vertex;
#endif
#if ESTIMATE_METHOD == SAMPLING
    std::set<uint32_t> sample;

    // Sampling factor = 1/sampling_factor, so 2 = 50%, 4 = 25%, etc.
    static const uint32_t sampling_factor = 128;

    // RNG initialization
    std::random_device r;
#endif

    // Number of nodes and number of labels.
    const uint32_t N, L;

    // Calculate in and output nodes
    static const bool calculate_in_and_out = false;

public:
    explicit SimpleEstimator(std::shared_ptr<SimpleGraph> &g);
    ~SimpleEstimator() = default;

    void prepare() override;
    cardStat estimate(RPQTree *q) override;

#if ESTIMATE_METHOD == PATH_PROBABILITY
    // Path probability:
    void prepareProbability();
    cardStat estimateProbability(RPQTree *q);
    void convertQuery(RPQTree *q, std::vector<uint32_t> &query);
    float calcProb(std::vector<uint32_t> query);
    template <typename T> void countPaths(std::vector<T> &path, uint32_t node);
#elif ESTIMATE_METHOD == SAMPLING
    // sampling:
    void prepareSampling();
    cardStat estimateSampling(RPQTree *q);
#endif
#if (ESTIMATE_METHOD == SAMPLING) || (ESTIMATE_METHOD == BRUTE_FORCE)
    // brute force:
    void prepareBruteForce();
    cardStat estimateBruteForce(RPQTree *q);

    std::vector<std::string> parseQuery(RPQTree *q);
    std::string treeToString(const RPQTree *q) const;
    int subEstimateBruteForce(const std::vector<std::string>& path, uint32_t node, bool calculate_start_vertices);
#endif
};


#endif //QS_SIMPLEESTIMATOR_H
