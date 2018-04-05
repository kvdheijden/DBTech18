//
// Created by Nikolay Yakovets on 2018-02-01.
//

#ifndef QS_SIMPLEESTIMATOR_H
#define QS_SIMPLEESTIMATOR_H

#include <vector>
#include <set>
#include <random>
#include <memory>
#include <sstream>

#include "Estimator.h"
#include "SimpleGraph.h"

////////////////////////////////
/// Variable Dimension Array ///
////////////////////////////////
template <typename T, size_t N> struct dimArr : public std::vector<std::pair<dimArr<T, N-1>, T>> {
    void init(size_t n, T val);
};
template <typename T> struct dimArr<T, 1> : public std::vector<T> {
    void init(size_t n, T val);
};
template <typename T> struct dimArr<T, 0> {dimArr() = delete;};

class EstimatorImpl : public Estimator {
protected:
    std::shared_ptr<SimpleGraph> graph;

    // Number of nodes and number of labels.
    const uint32_t N, L;

    // Calculate in and output nodes
    static const bool calculate_in_and_out = false;
public:
    explicit EstimatorImpl(std::shared_ptr<SimpleGraph> &g);

    virtual ~EstimatorImpl() = default;
    void query_to_vec(RPQTree * query, std::vector<uint32_t>& vec);
};

////////////////////////
/// Simple Estimator ///
////////////////////////
class SimpleEstimator : public Estimator {
protected:

    static const enum {
        PATH_PROBABILITY,
        SAMPLING,
        BRUTE_FORCE
    } estimate_method = PATH_PROBABILITY;

public:
    explicit SimpleEstimator(std::shared_ptr<SimpleGraph> &g);

    virtual ~SimpleEstimator();

    void prepare() override;
    cardStat estimate(RPQTree *q) override;

    EstimatorImpl *impl;
};

////////////////////////
/// Path Probability ///
////////////////////////
class PathProbEstimator : public EstimatorImpl {
protected:
    // Number of dimensions in pathProb.
    static const uint32_t D = 1;

    // Path probability:
    std::vector<uint32_t> nodesWithOutLabel;
    std::vector<uint32_t> nodesWithInLabel;
    dimArr<float, D> pathProbabilities;
public:
    explicit PathProbEstimator(std::shared_ptr<SimpleGraph> &g);
    ~PathProbEstimator() override = default;

    void prepare() override;

    cardStat estimate(RPQTree *q) override;

    template <size_t S> float calcProbRecursive(const std::vector<uint32_t>& query, const dimArr<float, S>& probabilities);
    template <size_t S> float calcProb(std::vector<uint32_t> query, const dimArr<float, S>& probabilities);
    template <size_t S> void countPaths(dimArr<uint32_t, S> &path, uint32_t node);
    template <size_t S> void calculatePathProbabilities(dimArr<float, S>& labelProbabilities, const dimArr<uint32_t, S>& labelCounts);
};

///////////////////
/// Brute Force ///
///////////////////
class BruteForceEstimator : public EstimatorImpl {
protected:
    std::vector<std::vector<std::pair<uint32_t, uint32_t>>> summary;
    std::vector<std::vector<std::pair<uint32_t, uint32_t>>> r_summary;
    uint32_t n_start_vertices;
    std::set<uint32_t> unique_end_vertices;
    std::set<uint32_t> unique_end_vertices_per_vertex;
public:
    explicit BruteForceEstimator(std::shared_ptr<SimpleGraph> &g);
    ~BruteForceEstimator() override = default;

    void prepare() override;

    cardStat estimate(RPQTree *q) override;

    std::vector<std::string> parseQuery(RPQTree *q);
    std::string treeToString(const RPQTree *q) const;
    int subEstimateBruteForce(const std::vector<std::string>& path, uint32_t node, bool calculate_start_vertices);

};

////////////////
/// Sampling ///
////////////////
class SamplingEstimator : public BruteForceEstimator {
protected:
    std::set<uint32_t> sample;

    // Sampling factor = 1/sampling_factor, so 2 = 50%, 4 = 25%, etc.
    static const uint32_t sampling_factor = 128;

    // RNG initialization
    std::random_device r;
public:
    explicit SamplingEstimator(std::shared_ptr<SimpleGraph> &g);
    ~SamplingEstimator() override = default;

    void prepare() override;

    cardStat estimate(RPQTree *q) override;

};

#endif //QS_SIMPLEESTIMATOR_H
