//
// Created by Nikolay Yakovets on 2018-02-02.
//

#include "SimpleEstimator.h"
#include "SimpleEvaluator.h"

SimpleEvaluator::SimpleEvaluator(std::shared_ptr<SimpleGraph> &g) {

    // works only with SimpleGraph
    graph = g;
    est = nullptr; // estimator not attached by default
}

void SimpleEvaluator::attachEstimator(std::shared_ptr<SimpleEstimator> &e) {
    est = e;
}

void SimpleEvaluator::prepare() {

    // if attached, prepare the estimator
    if(est != nullptr) est->prepare();

    // prepare other things here.., if necessary

}

cardStat SimpleEvaluator::computeStats(std::shared_ptr<SimpleGraph> &g) {

    cardStat stats {};

    for(int source = 0; source < g->getNoVertices(); source++) {
        if(!g->adj[source].empty()) stats.noOut++;
    }

    stats.noPaths = g->getNoDistinctEdges();

    for(int target = 0; target < g->getNoVertices(); target++) {
        if(!g->reverse_adj[target].empty()) stats.noIn++;
    }

    return stats;
}

std::shared_ptr<SimpleGraph> project(uint32_t projectLabel, bool inverse, std::shared_ptr<SimpleGraph> &in) {

    auto out = std::make_shared<SimpleGraph>(in->getNoVertices());
    out->setNoLabels(in->getNoLabels());

    if(!inverse) {
        // going forward
        for(uint32_t source = 0; source < in->getNoVertices(); source++) {
            for (auto labelTarget : in->adj[source]) {

                auto label = labelTarget.first;
                auto target = labelTarget.second;

                if (label == projectLabel)
                    out->addEdge(source, target, label);
            }
        }
    } else {
        // going backward
        for(uint32_t source = 0; source < in->getNoVertices(); source++) {
            for (auto labelTarget : in->reverse_adj[source]) {

                auto label = labelTarget.first;
                auto target = labelTarget.second;

                if (label == projectLabel)
                    out->addEdge(source, target, label);
            }
        }
    }

    return out;
}

std::shared_ptr<SimpleGraph> join(std::shared_ptr<SimpleGraph> &left, std::shared_ptr<SimpleGraph> &right) {

    auto out = std::make_shared<SimpleGraph>(left->getNoVertices());
    out->setNoLabels(1);

    for(uint32_t leftSource = 0; leftSource < left->getNoVertices(); leftSource++) {
        for (auto labelTarget : left->adj[leftSource]) {

            int leftTarget = labelTarget.second;
            // try to join the left target with right source
            for (auto rightLabelTarget : right->adj[leftTarget]) {

                auto rightTarget = rightLabelTarget.second;
                out->addEdge(leftSource, rightTarget, 0);

            }
        }
    }

    return out;
}

std::shared_ptr<ProjectionAlgorithm> SimpleEvaluator::find_best_projection_algorithm(RPQTree *query) {
    cardStat result = est->estimate(query);
    // TODO: Decide which projection algorithm to use
    return std::make_shared<ProjectionAlgorithm>(result.noPaths, query->data, this->graph, &project);
}

std::shared_ptr<JoiningAlgorithm> SimpleEvaluator::find_best_joining_algorithm(std::shared_ptr<QueryPlan> P1, std::shared_ptr<QueryPlan> P2) {
    float cost = P1->cost() + P2->cost() + 1000;
    return std::make_shared<JoiningAlgorithm>(cost, P1, P2, &join);
}

constexpr int mask = (1 << ((sizeof(int) * 8) - 1));

std::shared_ptr<QueryPlan> SimpleEvaluator::find_best_plan(const std::vector<int>& S) {
    if(best_plan.find(S) != best_plan.end()) {
        return best_plan[S];
    }

    // Need to calculate best plan
    if(S.size() == 1) {
        int v = S[0] & ~mask;
        std::string query_string = std::to_string(v);

        if(S[0] & mask) {
            query_string += '-';
        } else {
            query_string += '+';
        }
        RPQTree query(query_string, nullptr, nullptr);

        std::shared_ptr<ProjectionAlgorithm> A = find_best_projection_algorithm(&query);
        best_plan[S] = A;
    } else {
        for(auto it = S.begin() + 1; it < S.end(); it++) {
            std::vector<int> S1(S.begin(), it);
            std::vector<int> S2(it, S.end());
            std::shared_ptr<QueryPlan> P1 = find_best_plan(S1);
            std::shared_ptr<QueryPlan> P2 = find_best_plan(S2);
            std::shared_ptr<JoiningAlgorithm> A = find_best_joining_algorithm(P1, P2);

            if(best_plan.find(S) == best_plan.end()) {
                best_plan[S] = A;
                continue;
            }

            if(A->cost() < best_plan[S]->cost()) {
                best_plan[S] = A;
            }
        }
    }
    return best_plan[S];
}

void SimpleEvaluator::query_to_vec(RPQTree * query, std::vector<int>& vec) {
    if(query->isLeaf()) {
        const char *c_str = query->data.c_str();
        char *end;
        int label = std::strtol(c_str, &end, 10);
        if(*end == '-')
            label |= mask;
        vec.push_back(label);
    } else if (query->isUnary()) {
        query_to_vec(query->left, vec);
    } else {
        query_to_vec(query->left, vec);
        query_to_vec(query->right, vec);
    }
}

cardStat SimpleEvaluator::evaluate(RPQTree *query) {
    std::vector<int> flat_query;
    query_to_vec(query, flat_query);
    std::shared_ptr<QueryPlan> best_plan = find_best_plan(flat_query);

    std::shared_ptr<SimpleGraph> res = best_plan->execute();

    return SimpleEvaluator::computeStats(res);
}

ProjectionAlgorithm::ProjectionAlgorithm(float cost, const std::string &query_string, std::shared_ptr<SimpleGraph> graph,
                                         std::shared_ptr<SimpleGraph> (*project)(uint32_t, bool, std::shared_ptr<SimpleGraph> &))
        : QueryPlan(cost), label(query_string), graph(std::move(graph)), project(project) {}

std::shared_ptr<SimpleGraph> ProjectionAlgorithm::execute() {
    const char *c_str = label.c_str();
    char *end;
    uint32_t projectLabel = std::strtoul(c_str, &end, 10);

    return project(projectLabel, *end == '-', graph);
}

JoiningAlgorithm::JoiningAlgorithm(float cost, std::shared_ptr<QueryPlan> P1, std::shared_ptr<QueryPlan> P2,
                                   std::shared_ptr<SimpleGraph> (*join)(std::shared_ptr<SimpleGraph> &, std::shared_ptr<SimpleGraph> &))
        : QueryPlan(cost), P1(std::move(P1)), P2(std::move(P2)), join(join) {}

std::shared_ptr<SimpleGraph> JoiningAlgorithm::execute() {
    std::shared_ptr<SimpleGraph> left = P1->execute();
    std::shared_ptr<SimpleGraph> right = P2->execute();
    return join(left, right);
}
