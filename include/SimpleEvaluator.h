//
// Created by Nikolay Yakovets on 2018-02-02.
//

#ifndef QS_SIMPLEEVALUATOR_H
#define QS_SIMPLEEVALUATOR_H


#include <memory>
#include <iostream>
#include <unordered_set>
#include <cmath>
#include "SimpleGraph.h"
#include "RPQTree.h"
#include "Evaluator.h"
#include "Graph.h"

class SimpleEvaluator : public Evaluator {

    std::shared_ptr<SimpleGraph> graph;
    std::shared_ptr<SimpleEstimator> est;

// CACHING   // Unordered (hash)map for caching evaluations.
// CACHING   std::unordered_map<std::string, std::shared_ptr<SimpleGraph>> evaluationCache;

    //map<edge, vector<pair<vertex, vector<edge>>>
    std::unordered_map<uint32_t, std::vector<const SimpleEdge*>> edge_index;

public:

    explicit SimpleEvaluator(std::shared_ptr<SimpleGraph> &g);
    ~SimpleEvaluator() = default;

    void prepare() override ;
    cardStat evaluate(RPQTree *query) override ;

    void attachEstimator(std::shared_ptr<SimpleEstimator> &e);

    std::shared_ptr<SimpleGraph> evaluate_aux(RPQTree *q);
    std::shared_ptr<SimpleGraph> project(uint32_t label, bool inverse, std::shared_ptr<SimpleGraph> &g);
    std::shared_ptr<SimpleGraph> join(std::shared_ptr<SimpleGraph> &left, std::shared_ptr<SimpleGraph> &right);
    cardStat computeStats(std::shared_ptr<SimpleGraph> &g);

    RPQTree* convertEfficientAST(RPQTree *q);
    RPQTree* generateEfficientAST(std::vector<uint32_t> &query, uint32_t &totalCost, std::unordered_map<std::string, uint32_t> &ec);

    uint32_t convertLabelToInt(std::string label);
    std::string convertIntToLabel(uint32_t i);
    std::string vecToString(std::vector<uint32_t> vec);

    std::shared_ptr<SimpleGraph> invertedJoin(std::shared_ptr<SimpleGraph> &left, std::shared_ptr<SimpleGraph> &right);
};


#endif //QS_SIMPLEEVALUATOR_H
