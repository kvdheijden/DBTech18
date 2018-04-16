//
// Created by Nikolay Yakovets on 2018-02-02.
//

#include "SimpleEvaluator.h"

#include <cmath>

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

cardStat SimpleEvaluator::computeStats(std::shared_ptr<InterGraph> &g) {
    std::set<uint32_t> starts, ends;

    for(const auto& edge : g->edges) {
        starts.insert(edge.first);
        ends.insert(edge.second);
    }

    return {
            starts.size(),
            g->getNoDistinctEdges(),
            ends.size()
    };
}

std::shared_ptr<InterGraph> project(uint32_t projectLabel, bool inverse, std::shared_ptr<SimpleGraph> &in) {

    auto out = std::make_shared<InterGraph>(in->getNoVertices());

    const auto& list = (inverse ? in->edge_index_inverse[projectLabel] : in->edge_index[projectLabel]);

    for (auto edge : list)
    {
        out->addEdge(edge.first, edge.second, projectLabel);
    }

    return out;
}

void advance(std::vector<std::pair<uint32_t, uint32_t>>::iterator& begin,
             std::vector<std::pair<uint32_t, uint32_t>>::iterator& ptr,
             const std::vector<std::pair<uint32_t, uint32_t>>::iterator& end,
             uint32_t& key) {
    begin = ptr;
    key = ptr->first;

    while(ptr != end && ptr->first == key) {
        ptr++;
    }
}

void sort(std::vector<std::pair<uint32_t, uint32_t>>& in) {
    std::sort(in.begin(), in.end(), [](const std::pair<uint32_t, uint32_t>& a, const std::pair<uint32_t, uint32_t>& b){
        if(a.first == b.first) {
            return a.second < b.second;
        }
        return a.first < b.first;
    });
}

std::shared_ptr<InterGraph> sort_merge_join(std::shared_ptr<InterGraph> &left, std::shared_ptr<InterGraph> &right) {
    auto output = std::make_shared<InterGraph>(left->getNoVertices());

    // Transform the left edge vector into <target, source> pairs, so we can sort/compare on element.first in both left and right
    std::transform(left->edges.begin(), left->edges.end(), left->edges.begin(), [](const std::pair<uint32_t, uint32_t>& e){
        return std::make_pair(e.second, e.first);
    });

    // Sort left and right edge vectors
    sort(left->edges);
    sort(right->edges);

    auto left_begin = left->edges.begin();
    auto right_begin = right->edges.begin();
    auto p_left = left->edges.begin();
    auto p_right = right->edges.begin();
    auto left_end = left->edges.end();
    auto right_end = right->edges.end();

    // Define keys and subsets
    uint32_t left_key, right_key;

    // Initial advance first element
    advance(left_begin, p_left, left_end, left_key);
    advance(right_begin, p_right, right_end, right_key);

    while (left_begin != left_end && right_begin != right_end) {
        if(left_key == right_key) {

            // Naive cartesian product
            for(auto it1 = left_begin; it1 != p_left; it1++) {
                for(auto it2 = right_begin; it2 != p_right; it2++) {
                    output->addEdge(it1->second, it2->second, 0);
                }
            }

            // Advance both
            advance(left_begin, p_left, left_end, left_key);
            advance(right_begin, p_right, right_end, right_key);
        } else if (left_key < right_key) {
            // Advance left, since it's smaller than right
            advance(left_begin, p_left, left_end, left_key);
        } else {
            // Advance right, since it's smaller than left
            advance(right_begin, p_right, right_end, right_key);
        }
    }

    return output;
}

std::shared_ptr<InterGraph> nested_loops_join(std::shared_ptr<InterGraph> &left, std::shared_ptr<InterGraph> &right) {

    auto out = std::make_shared<InterGraph>(left->getNoVertices());

    // For each tuple r in R do
    for(const auto& left_edge : left->edges) {
        uint32_t left_source = left_edge.first;
        uint32_t left_target = left_edge.second;

        // For each tuple s in S do
        for(const auto& right_edge : right->edges) {
            uint32_t right_source = right_edge.first;
            uint32_t right_target = right_edge.second;

            // If r and s satisfy the join condition
            if(left_target == right_source)

                // Then output the tuple <r,s>
                out->addEdge(left_source, right_target, 0);
        }
    }

    return out;
}

std::shared_ptr<ProjectionAlgorithm> SimpleEvaluator::find_best_projection_algorithm(RPQTree *query) {
    cardStat result = est->estimate(query);
    return std::make_shared<ProjectionAlgorithm>(result.noPaths, query->data, this->graph, &project);
}

std::shared_ptr<JoiningAlgorithm> SimpleEvaluator::find_best_joining_algorithm(std::shared_ptr<QueryPlan> P1, std::shared_ptr<QueryPlan> P2) {
    // TODO: Plan
    const float T_r = P1->cost, T_s = P2->cost;
    return std::make_shared<JoiningAlgorithm>(T_r + T_s, P1, P2, &sort_merge_join);
}

std::shared_ptr<QueryPlan> SimpleEvaluator::find_best_plan(const std::vector<uint32_t>& S) {
    if(best_plan.find(S) != best_plan.end()) {
        return best_plan[S];
    }

    // Need to calculate best plan
    if(S.size() == 1) {
        std::string query_string;

        if(S[0] < graph->getNoLabels()) {
            query_string += std::to_string(S[0]) + '+';
        } else {
            query_string += std::to_string(S[0] - graph->getNoLabels()) + '-';
        }
        RPQTree query(query_string, nullptr, nullptr);

        std::shared_ptr<ProjectionAlgorithm> A = find_best_projection_algorithm(&query);
        best_plan[S] = A;
    } else {
        for(auto it = S.begin() + 1; it < S.end(); it++) {
            std::vector<uint32_t> S1(S.begin(), it);
            std::vector<uint32_t> S2(it, S.end());
            std::shared_ptr<QueryPlan> P1 = find_best_plan(S1);
            std::shared_ptr<QueryPlan> P2 = find_best_plan(S2);
            std::shared_ptr<JoiningAlgorithm> A = find_best_joining_algorithm(P1, P2);

            if(best_plan.find(S) == best_plan.end()) {
                best_plan[S] = A;
                continue;
            }

            if(A->cost< best_plan[S]->cost) {
                best_plan[S] = A;
            }
        }
    }
    return best_plan[S];
}

cardStat SimpleEvaluator::evaluate(RPQTree *query) {
    std::vector<uint32_t> flat_query;
    est->impl->query_to_vec(query, flat_query);
    std::shared_ptr<QueryPlan> best_plan = find_best_plan(flat_query);

    std::shared_ptr<InterGraph> res = best_plan->execute();

    return SimpleEvaluator::computeStats(res);
}

ProjectionAlgorithm::ProjectionAlgorithm(float cost, const std::string &query_string, std::shared_ptr<SimpleGraph> graph,
                                         std::shared_ptr<InterGraph> (*project)(uint32_t, bool, std::shared_ptr<SimpleGraph> &))
        : QueryPlan(cost), label(query_string), graph(std::move(graph)), project(project) {}

std::shared_ptr<InterGraph> ProjectionAlgorithm::execute() {
    const char *c_str = label.c_str();
    char *end;
    uint32_t projectLabel = std::strtoul(c_str, &end, 10);

    return project(projectLabel, *end == '-', graph);
}

JoiningAlgorithm::JoiningAlgorithm(float cost, std::shared_ptr<QueryPlan> P1, std::shared_ptr<QueryPlan> P2,
                                   std::shared_ptr<InterGraph> (*join)(std::shared_ptr<InterGraph> &, std::shared_ptr<InterGraph> &))
        : QueryPlan(cost), P1(std::move(P1)), P2(std::move(P2)), join(join) {}

std::shared_ptr<InterGraph> JoiningAlgorithm::execute() {
    std::shared_ptr<InterGraph> left = P1->execute();
    std::shared_ptr<InterGraph> right = P2->execute();
    return join(left, right);
}
