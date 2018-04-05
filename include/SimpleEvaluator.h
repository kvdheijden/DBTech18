//
// Created by Nikolay Yakovets on 2018-02-02.
//

#ifndef QS_SIMPLEEVALUATOR_H
#define QS_SIMPLEEVALUATOR_H


#include <memory>
#include <cmath>
#include "SimpleGraph.h"
#include "RPQTree.h"
#include "Evaluator.h"
#include "Graph.h"

class SimpleEvaluator;

class QueryPlan {
private:
    const float c;
protected:
    explicit QueryPlan(float cost) : c(cost) {};
public:
    virtual std::shared_ptr<SimpleGraph> execute() = 0;
    virtual float cost() {
        return c;
    };
};

class ProjectionAlgorithm : public QueryPlan {
private:
    std::string label;
    std::shared_ptr<SimpleGraph> graph;
    std::shared_ptr<SimpleGraph> (*project)(uint32_t projectLabel, bool inverse, std::shared_ptr<SimpleGraph> &in);
public:
    ProjectionAlgorithm(float cost, const std::string& query_string, std::shared_ptr<SimpleGraph> graph, std::shared_ptr<SimpleGraph> (*project)(uint32_t, bool, std::shared_ptr<SimpleGraph> &));
    std::shared_ptr<SimpleGraph> execute() override;
};

class JoiningAlgorithm : public QueryPlan {
private:
    std::shared_ptr<QueryPlan> P1;
    std::shared_ptr<QueryPlan> P2;
    std::shared_ptr<SimpleGraph> (*join)(std::shared_ptr<SimpleGraph> &left, std::shared_ptr<SimpleGraph> &right);
public:
    JoiningAlgorithm(float cost, std::shared_ptr<QueryPlan> P1, std::shared_ptr<QueryPlan> P2, std::shared_ptr<SimpleGraph> (*join)(std::shared_ptr<SimpleGraph> &, std::shared_ptr<SimpleGraph> &));
    std::shared_ptr<SimpleGraph> execute() override;
};

class SimpleEvaluator : public Evaluator {

    std::map<std::vector<int>, std::shared_ptr<QueryPlan>> best_plan;

    std::shared_ptr<SimpleGraph> graph;
    std::shared_ptr<SimpleEstimator> est;

public:

    explicit SimpleEvaluator(std::shared_ptr<SimpleGraph> &g);
    ~SimpleEvaluator() = default;

    void prepare() override ;
    cardStat evaluate(RPQTree *query) override ;

    void attachEstimator(std::shared_ptr<SimpleEstimator> &e);

    static cardStat computeStats(std::shared_ptr<SimpleGraph> &g);

    std::shared_ptr<ProjectionAlgorithm> find_best_projection_algorithm(RPQTree *query);
    std::shared_ptr<JoiningAlgorithm> find_best_joining_algorithm(std::shared_ptr<QueryPlan> P1, std::shared_ptr<QueryPlan> P2);

    void query_to_vec(RPQTree * query, std::vector<int>& vec);
    std::shared_ptr<QueryPlan> find_best_plan(const std::vector<int>& S);
};


#endif //QS_SIMPLEEVALUATOR_H
