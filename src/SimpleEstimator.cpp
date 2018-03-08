//
// Created by Nikolay Yakovets on 2018-02-01.
//

#include <random>
#include <iostream>
#include <iterator>
#include <algorithm>
#include "SimpleGraph.h"
#include "SimpleEstimator.h"

// TODO, are the esimates definitely calculated correctly?
// DONE, is built in estimator definitely correct? Does not agree with our brute-force method... <- fixed by pr

// DONE, create appropriate deconstructor <-- This is not needed. We only use the stack, so no deconstruction is necessary

SimpleEstimator::SimpleEstimator(std::shared_ptr<SimpleGraph> &g) :
    N(g->getNoVertices()), L(g->getNoLabels()),
    nodesWithInLabel(g->getNoLabels(), 0), nodesWithOutLabel(g->getNoLabels(), 0),
    labelProbs(g->getNoLabels(), 0.0f), pathProbs3(g->getNoLabels() * 2, std::pair<std::vector<std::pair<std::vector<float>, float>>, float>(std::vector<std::pair<std::vector<float>, float>>(g->getNoLabels() * 2, std::pair<std::vector<float>, float>(std::vector<float>(g->getNoLabels() * 2, 0.0f), 0.0f)), 0.0f))
{
    // works only with SimpleGraph
    graph = g;
}

void SimpleEstimator::prepare() {
    if (estimateMethod == 0) {
        prepareFirst();
    } else if (estimateMethod == 1){
        prepareBruteForce();
    } else if (estimateMethod == 2){
        prepareSampling();
    }
}

void SimpleEstimator::prepareFirst() {

    // Total number of times a specific set of (at most three) labels occurs.
    std::vector<std::pair<std::vector<std::pair<std::vector<uint32_t>, uint32_t>>, uint32_t>> labels3(2 * L, std::pair<std::vector<std::pair<std::vector<uint32_t>, uint32_t>>, uint32_t>(std::vector<std::pair<std::vector<uint32_t>, uint32_t>>(2 * L, std::pair<std::vector<uint32_t>, uint32_t>(std::vector<uint32_t>(2 * L, 0), 0)), 0));

    // Go through all nodes and count the transition labels and number of nodes with specific outgoing transitions.
    for (uint32_t i = 0; i < N; i++) {
        countPaths(labels3, i);
    }

    // Keeps track of whether a certain label has already been seen for a certain node.
    std::vector<bool> seenLabelForNode(L, false);

    // Go through all nodes and count the transition labels and number of nodes with specific outgoing transitions.
    for ( const std::vector<std::pair<uint32_t,uint32_t>> &n : graph->adj ) {

        // Reset flags for which labels have already been seen for this node.
        for (auto &&i : seenLabelForNode) {
            i = false;
        }

        // Go through all transitions from this node.
        for ( const std::pair<uint32_t,uint32_t> &t : n ) {
            uint32_t label = t.first;
            if ( !seenLabelForNode[label] ) {
                nodesWithOutLabel[label]++;
                seenLabelForNode[label] = true;
            }
        }
    }

    // Go through all nodes and count the number of nodes with specific incoming transitions.
    for ( const std::vector<std::pair<uint32_t,uint32_t>> &n : graph->reverse_adj ) {

        // Reset flags for which labels have already been seen for this node.
        for (auto &&i : seenLabelForNode) {
            i = false;
        }

        // Go through all transitions to this node.
        for ( const std::pair<uint32_t,uint32_t> &t : n ) {
            uint32_t label = t.first;
            if ( !seenLabelForNode[label] ) {
                nodesWithInLabel[label]++;
                seenLabelForNode[label] = true;
            }
        }
    }

    // Turn counts into ~probabilites by dividing by N.
    for (int i = 0; i < 2 * L; i++) {
        pathProbs3[i].second = ((float)labels3[i].second)/((float)N);
        for (int j = 0; j < 2 * L; j++) {
            pathProbs3[i].first[j].second = ((float)labels3[i].first[j].second)/((float)N);
            for (int k = 0; k < 2 * L; k++) {
                pathProbs3[i].first[j].first[k] = ((float)labels3[i].first[j].first[k])/((float)N);
            }
        }
    }


    std::cout << std::endl << pathProbs3[9].first[8].first[17] * N;
}

template<typename T>
void SimpleEstimator::countPaths(std::vector<T>& path, uint32_t node) {
    // Get each transition from this node and call this function again.

    // Both adjacency matrices.
    std::vector<std::vector<std::pair<uint32_t,uint32_t>>>& adj = graph->adj;
    std::vector<std::vector<std::pair<uint32_t,uint32_t>>>& r_adj = graph->reverse_adj;

    // Get the transitions from both.
    std::vector<std::pair<uint32_t,uint32_t>>& ts = adj[node];
    std::vector<std::pair<uint32_t,uint32_t>>& r_ts = r_adj[node];

    // Go through all outgoing transitions from this node.
    for ( const std::pair<uint32_t,uint32_t> &t : ts ) {
        uint32_t label = t.first;
        countPaths(path[label].first, t.second);
        path[label].second++;
    }

    // Go through all incoming transitions from this node.
    for ( const std::pair<uint32_t,uint32_t> &t : r_ts ) {
        uint32_t label = t.first;
        countPaths(path[L+label].first, t.second);
        path[L+label].second++;
    }
}

template <>
void SimpleEstimator::countPaths<uint32_t>(std::vector<uint32_t>& path, uint32_t node) {
    // Both adjacency matrices.
    std::vector<std::vector<std::pair<uint32_t,uint32_t>>>& adj = graph->adj;
    std::vector<std::vector<std::pair<uint32_t,uint32_t>>>& r_adj = graph->reverse_adj;

    // Get the transitions from both.
    std::vector<std::pair<uint32_t,uint32_t>>& ts = adj[node];
    std::vector<std::pair<uint32_t,uint32_t>>& r_ts = r_adj[node];

    // Go through all outgoing transitions from this node.
    for ( const std::pair<uint32_t,uint32_t> &t : ts ) {
        uint32_t label = t.first;
        path[label]++;
    }

    // Go through all incoming transitions from this node.
    for ( const std::pair<uint32_t,uint32_t> &t : r_ts ) {
        uint32_t label = t.first;
        path[L+label]++;
    }
}

cardStat SimpleEstimator::estimate(RPQTree *q) {
    if (estimateMethod == 0) {
        return estimateFirst(q);
    } else if (estimateMethod == 1) {
        return estimateBruteForce(q);
    } else if (estimateMethod == 2) {
        return estimateSampling(q);
    }
}

cardStat SimpleEstimator::estimateFirst(RPQTree *q) {

    // Counts of potential start nodes and end nodes for this query.
    uint32_t potStartNodeCount, potEndNodeCount;

    // Probabilities for: the whole path, the path without start transition, without end transition.
    float pathProb, startPathProb, endPathProb;

    // Statistics to be reported at the end.
    uint32_t paths, startNodes, endNodes;

    auto leftmost = q;
    while ( leftmost->left != nullptr ) leftmost = leftmost->left;
    std::string firstLabel = leftmost->data;
    uint32_t firstLabelInt = std::stoul(firstLabel);

    auto rightmost = q;
    while ( rightmost->right != nullptr ) rightmost = rightmost->right;
    std::string lastLabel = rightmost->data;
    uint32_t lastLabelInt = std::stoul(lastLabel);

    if ( firstLabel[firstLabel.size()-1] == '+' ) {
        potStartNodeCount = nodesWithOutLabel[firstLabelInt];
    } else {
        potStartNodeCount = nodesWithInLabel[firstLabelInt];
    }

    if ( lastLabel[firstLabel.size()-1] == '+') {
        potEndNodeCount = nodesWithInLabel[lastLabelInt];
    } else {
        potEndNodeCount = nodesWithOutLabel[lastLabelInt];
    }

    // Traverse tree in order to store query in vector format.
    // Then calculate path probs for:
    // - Once completely
    // - Once without starting node
    // - Once without ending node
    //
    // During prob calc, know how many dimensions D deep pathCount is.
    // Get an amount of transitions at most D, look up probability and multiply with previous probability.

    std::vector<uint32_t> query;
    convertQuery(q, query);
    pathProb = calcProb(query);
    std::vector<uint32_t> startQuery = query;
    std::vector<uint32_t> endQuery = query;
    startQuery.erase(startQuery.begin());
    startPathProb = calcProb(startQuery);
    endQuery.pop_back();
    endPathProb = calcProb(endQuery);

    // To obtain final metrics, multiply path prob by N and start/end by the respective potNodeCounts.
    paths = (uint32_t)(pathProb * ((float) N));
    startNodes = (uint32_t)(startPathProb * ((float) potStartNodeCount));
    endNodes = (uint32_t)(endPathProb * ((float) potEndNodeCount));

    // TODO, (not necessary) fix the start and end node estimations

    return cardStat {startNodes, paths, endNodes};
}

float SimpleEstimator::calcProb(std::vector<uint32_t> query) {

    int qs = query.size();
    float prob = 1;

    // ADAPT
    if (qs == 0) {
        return prob;
    } else if (qs == 1) {
        uint32_t t1 = query[0];
        prob = pathProbs3[t1].second;
        return prob;
    } else if (qs == 2) {
        uint32_t t1 = query[0];
        uint32_t t2 = query[1];
        prob = pathProbs3[t1].first[t2].second;
        return prob;
    } else {
        uint32_t t1 = query[0];
        uint32_t t2 = query[1];
        uint32_t t3 = query[2];
        prob = pathProbs3[t1].first[t2].first[t3];

        std::cout << std::endl << t1 << t2 << t3 << prob;

        for (int i = 0; i < qs-3; ++i) {
            t1 = query[i+1];
            t2 = query[i+2];
            t3 = query[i+3];
            prob *= (pathProbs3[t1].first[t2].first[t3] / pathProbs3[t1].first[t2].second);
        }

        return prob;
    }
}

// Converts query tree to vector with query parts in order. 1+/2+/0-/3- with L = 4 is coded as 1,0,0+L,3+L = 1,0,4,7.
void SimpleEstimator::convertQuery(RPQTree *q, std::vector<uint32_t>& query) {
    // First go to left leaf. If there is any useful data, append it to the query. Then go to right leaf.

    if (q == nullptr) {
        return;
    }

    convertQuery(q->left, query);

    std::string rootLabel = q->data;
    if ( rootLabel != "/" ) {
        unsigned int rootLabelInt = std::stoul(rootLabel);
        if (rootLabel[rootLabel.size()-1] == '+') {
            query.push_back(rootLabelInt);
        } else {
            query.push_back(rootLabelInt + L);
        }
    }

    convertQuery(q->right, query);
}

void SimpleEstimator::prepareBruteForce()
{
    summary.resize(graph->getNoVertices());

    for (size_t node = 0; node < graph->adj.size(); node++)
    {
        summary[node].resize(2);
        summary[node][0].insert(summary[node][0].end(),
                                graph->adj[node].begin(),
                                graph->adj[node].end());
    }

    for (size_t node = 0; node < graph->reverse_adj.size(); node++)
    {
        summary[node][1].insert(summary[node][1].end(), graph->reverse_adj[node].begin(), graph->reverse_adj[node].end());
    }
}

void SimpleEstimator::prepareSampling()
{
    auto max_size = graph->getNoVertices();
    auto total_samples = (int)floor(max_size/sampling_factor);

    for (;sample.size() <= total_samples;)
    {
        sample.insert(rand() % static_cast<uint32_t>(max_size));
    }

    summary.resize(graph->getNoVertices());

    for (size_t node = 0; node < graph->adj.size(); node++)
    {
        summary[node].resize(2);
        summary[node][0].insert(summary[node][0].end(),
                                graph->adj[node].begin(),
                                graph->adj[node].end());
    }

    for (size_t node = 0; node < graph->reverse_adj.size(); node++)
    {
        summary[node][1].insert(summary[node][1].end(), graph->reverse_adj[node].begin(), graph->reverse_adj[node].end());
    }
}

cardStat SimpleEstimator::estimateBruteForce(RPQTree *q)
{
    this->a_start_vertices = 0;
    this->unique_end_vertices.clear();
    uint32_t total = 0;
    auto query = treeToString(q);

    std::stringstream ss(query);
    std::string item;
    std::vector<std::string> path;

    while (std::getline(ss, item, '/'))
    {
        path.push_back(item);
    }

    if (q != nullptr) {
        for (size_t node = 0; node < summary.size(); node++) {
            this->unique_end_vertices_per_vertex.clear();
            subestimateBruteForce(path, node, calculate_in_and_out);
            total += this->unique_end_vertices_per_vertex.size();
        }
    }

    if (calculate_in_and_out) {
        return cardStat {this->a_start_vertices, total, static_cast<uint32_t>(this->unique_end_vertices.size())};
    }
    else
    {
        return cardStat {0, total, 0};
    }
}

cardStat SimpleEstimator::estimateSampling(RPQTree *q)
{
    if (sampling_factor <= 1)
    {
        return estimateBruteForce(q);
    }

    this->a_start_vertices = 0;
    this->unique_end_vertices.clear();
    uint32_t total = 0;
    auto query = treeToString(q);

    std::stringstream ss(query);
    std::string item;
    std::vector<std::string> path;

    while (std::getline(ss, item, '/'))
    {
        path.push_back(item);
    }

    if (q != nullptr)
    {
        for (auto i = sample.begin(); i != sample.end(); i++)
        {
            this->unique_end_vertices_per_vertex.clear();
            subestimateBruteForce(path, *i, calculate_in_and_out);
            total += this->unique_end_vertices_per_vertex.size();
        }
    }

    total *= sampling_factor;

    if (calculate_in_and_out)
    {
        return cardStat {this->a_start_vertices, total, static_cast<uint32_t>(this->unique_end_vertices.size())};
    }
    else
    {
        return cardStat {0, total, 0};
    }
}

int SimpleEstimator::subestimateBruteForce(std::vector<std::string> path, uint32_t node, bool calculate_start_vertices)
{
    int total = 0;

    if (path.empty())
    {
        if (calculate_in_and_out)
        {
            this->unique_end_vertices.insert(node);
        }

        this->unique_end_vertices_per_vertex.insert(node);
        return 1;
    }

    auto subpath = path;
    subpath.erase(subpath.begin());

    int dir = 0;

    if (path[0][1] == '-')
    {
        dir = 1;
    }

    for (auto edge : summary[node][dir])
    {
        char *c;
        if (edge.first == std::strtol(path[0].c_str(), &c, 10))
        {
            total += subestimateBruteForce(subpath, edge.second, false);

            if (calculate_start_vertices && total > 0)
            {
                this->a_start_vertices++;
                calculate_start_vertices = false;
            }
        }
    }

    return total;
}

std::string SimpleEstimator::treeToString(RPQTree *q)
{
    if (q == nullptr)
    {
        return "";
    }

    return treeToString(q->left) + q->data + treeToString(q->right);
}