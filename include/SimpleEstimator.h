//
// Created by Nikolay Yakovets on 2018-02-01.
//

#ifndef QS_SIMPLEESTIMATOR_H
#define QS_SIMPLEESTIMATOR_H

#define PATH_PROBABILITY    (0x01)
#define SAMPLING            (0x02)
#define BRUTE_FORCE         (0x03)

#ifndef ESTIMATE_METHOD
#define ESTIMATE_METHOD     PATH_PROBABILITY
#endif

#include <memory>

#if ESTIMATE_METHOD == PATH_PROBABILITY
#include <vector>
#elif (ESTIMATE_METHOD == SAMPLING) || (ESTIMATE_METHOD == BRUTE_FORCE)
#include <vector>
#include <set>
#include <random>
#endif

#include "Estimator.h"
#include "SimpleGraph.h"

#if ESTIMATE_METHOD == PATH_PROBABILITY
template <typename T, size_t N> struct dimArr : public std::vector<std::pair<dimArr<T, N-1>, T>> {
    void init(size_t n, T val);
};
template <typename T> struct dimArr<T, 1> : public std::vector<T> {
    void init(size_t n, T val);
};
template <typename T> struct dimArr<T, 0> {dimArr() = delete;};
#endif

class SimpleEstimator : public Estimator {
private:
    std::shared_ptr<SimpleGraph> graph;

#if ESTIMATE_METHOD == PATH_PROBABILITY
    // Number of dimensions in pathProb.
    static const uint32_t D = 3;

    // Path probability:
    std::vector<uint32_t> nodesWithOutLabel;
    std::vector<uint32_t> nodesWithInLabel;
    dimArr<float, D> pathProbabilities;
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
    template <size_t S> float calcProbRecursive(const std::vector<uint32_t>& query, const dimArr<float, S>& probabilities);
    template <size_t S> float calcProb(std::vector<uint32_t> query, const dimArr<float, S>& probabilities);
    template <size_t S> void countPaths(dimArr<uint32_t, S> &path, uint32_t node);
    template <size_t S> void calculatePathProbabilities(dimArr<float, S>& labelProbabilities, const dimArr<uint32_t, S>& labelCounts);
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
