//
// Created by Nikolay Yakovets on 2018-02-02.
//

#ifndef QS_SIMPLEEVALUATOR_H
#define QS_SIMPLEEVALUATOR_H

#include <memory>
#include <map>
#include <unordered_set>

#include "Evaluator.h"
#include "SimpleGraph.h"
#include "SimpleEstimator.h"

class SimpleEvaluator;

class QueryPlan {
private:
protected:
    explicit QueryPlan(float cost) : cost(cost) {};
public:
    virtual std::shared_ptr<InterGraph> execute() = 0;
    float cost;
};

class ProjectionAlgorithm : public QueryPlan {
private:
    std::string label;
    std::shared_ptr<SimpleGraph> graph;
    std::shared_ptr<InterGraph> (*project)(uint32_t projectLabel, bool inverse, std::shared_ptr<SimpleGraph> &in);
public:
    ProjectionAlgorithm(float cost, const std::string& query_string, std::shared_ptr<SimpleGraph> graph, std::shared_ptr<InterGraph> (*project)(uint32_t, bool, std::shared_ptr<SimpleGraph> &));
    std::shared_ptr<InterGraph> execute() override;
};

class JoiningAlgorithm : public QueryPlan {
private:
    std::shared_ptr<QueryPlan> P1;
    std::shared_ptr<QueryPlan> P2;
    std::shared_ptr<InterGraph> (*join)(std::shared_ptr<InterGraph> &left, std::shared_ptr<InterGraph> &right);
public:
    JoiningAlgorithm(float cost, std::shared_ptr<QueryPlan> P1, std::shared_ptr<QueryPlan> P2, std::shared_ptr<InterGraph> (*join)(std::shared_ptr<InterGraph> &, std::shared_ptr<InterGraph> &));
    std::shared_ptr<InterGraph> execute() override;
};

class SimpleEvaluator : public Evaluator {

    std::map<std::vector<uint32_t>, std::shared_ptr<QueryPlan>> best_plan;

    std::shared_ptr<SimpleGraph> graph;
    std::shared_ptr<SimpleEstimator> est;
public:

    explicit SimpleEvaluator(std::shared_ptr<SimpleGraph> &g);
    ~SimpleEvaluator() = default;

    void prepare() override ;
    cardStat evaluate(RPQTree *query) override ;

    void attachEstimator(std::shared_ptr<SimpleEstimator> &e);

    static cardStat computeStats(std::shared_ptr<InterGraph> &g);

    std::shared_ptr<ProjectionAlgorithm> find_best_projection_algorithm(RPQTree *query);
    std::shared_ptr<JoiningAlgorithm> find_best_joining_algorithm(std::shared_ptr<QueryPlan> P1, std::shared_ptr<QueryPlan> P2);

    std::shared_ptr<QueryPlan> find_best_plan(const std::vector<uint32_t>& S);

    std::unordered_map<uint32_t, std::vector<std::pair<uint32_t, uint32_t>>> edge_index;
    std::unordered_map<uint32_t, std::vector<std::pair<uint32_t, uint32_t>>> edge_index_inverse;
};


#endif //QS_SIMPLEEVALUATOR_H
