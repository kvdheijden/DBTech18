//
// Created by Nikolay Yakovets on 2018-02-01.
//

#include "SimpleGraph.h"
#include "SimpleEstimator.h"

// TODO, are the esimates definitely calculated correctly?
// TODO, is built in estimator definitely correct? Does not agree with our brute-force method...
// TODO, create appropriate deconstructor

SimpleEstimator::SimpleEstimator(std::shared_ptr<SimpleGraph> &g){

    // works only with SimpleGraph
    graph = g;
}

void SimpleEstimator::prepare() {
    if (estimateMethod == 0) {
        prepareFirst();
    } else {
        prepareBruteForce();
    }
}

void SimpleEstimator::prepareFirst() {

    // Total number of nodes and number of different labels.
    N = (*graph).getNoVertices();
    L = (*graph).getNoLabels();

    // ADAPT
    // Total number of times a specific set of (at most three) labels occurs.
    std::vector<std::pair<std::vector<std::pair<std::vector<uint32_t>, uint32_t>>, uint32_t>> labels3;

    // ADAPT
    // Resize all vectors beforehand.
    labels3.resize(2*L);
    for (int i = 0; i < 2*L; i++) {
        labels3[i].first.resize(2*L);
        for (int j = 0; j < 2*L; j++) {
            labels3[i].first[j].first.resize(2*L);
        }
    }

    // Go through all nodes and count the transition labels and number of nodes with specific outgoing transitions.
    for ( int i = 0; i < N; i++) {
        countPaths(labels3, i);
        //std::cout << i << "/" << N << std::endl;
    }
    //std::cout << std::endl << "Number of occurences of 0+/0-/0+ = " << labels3[0].first[L].first[0] << std::endl;
    //std::cout << std::endl << "Number of occurences of 0+/0- = " << labels3[0].first[L].second << std::endl;

    // Total number of nodes with a specific outgoing label.
    nodesWithOutLabel.resize(L);
    // Total number of nodes with a specific incoming label.
    nodesWithInLabel.resize(L);
    // The ~probability for each label to occur (same for incoming and outgoing).
    labelProbs.resize(L);
    // Keeps track of whether a certain label has already been seen for a certain node.
    std::vector<bool> seenLabelForNode;
    seenLabelForNode.resize(L);

    // Go through all nodes and count the transition labels and number of nodes with specific outgoing transitions.
    auto adj = &(*graph).adj;
    for ( const auto &n : *adj ) {
        // Reset flags for which labels have already been seen for this node.
        for ( int i = 0; i < seenLabelForNode.size(); i++ ) {
            seenLabelForNode[i] = false;
        }
        // Go through all transitions from this node.
        for ( const auto &t : n ) {
            uint32_t label = t.first;
            if ( !seenLabelForNode[label] ) {
                nodesWithOutLabel[label]++;
                seenLabelForNode[label] = true;
            }
        }
    }

    // Go through all nodes and count the number of nodes with specific incoming transitions.
    auto reverse_adj = &(*graph).reverse_adj;
    for ( const auto &n : *reverse_adj ) {
        // Reset flags for which labels have already been seen for this node.
        for ( int i = 0; i < seenLabelForNode.size(); i++ ) {
            seenLabelForNode[i] = false;
        }
        // Go through all transitions to this node.
        for ( const auto &t : n ) {
            uint32_t label = t.first;
            if ( !seenLabelForNode[label] ) {
                nodesWithInLabel[label]++;
                seenLabelForNode[label] = true;
            }
        }
    }

    // ADAPT
    // Turn counts into ~probabilites by dividing by N.
    pathProbs3.resize(2*L);
    for (int i = 0; i < 2*L; i++) {
        pathProbs3[i].first.resize(2*L);
        pathProbs3[i].second = ((float)labels3[i].second)/((float)N);
        for (int j = 0; j < 2*L; j++) {
            pathProbs3[i].first[j].first.resize(2*L);
            pathProbs3[i].first[j].second = ((float)labels3[i].first[j].second)/((float)N);
            for (int k = 0; k < 2*L; k++) {
                pathProbs3[i].first[j].first[k] = ((float)labels3[i].first[j].first[k])/((float)N);
            }
        }
    }
}

// TODO, make this nice and recursive with template.
// ADAPT, add new func.

// Function that takes a vector of vectors. It goes through all connecting nodes and either:
// - Calls itself again, with the corresponding vector of the transition, to go through the other nodes' nodes.
// - Calls another variant of itself that takes a vector of ints that actually increases the counts.
// (does this automatically based on data type that is passed along)
// Copy paste this function and add another vector layer to expand functionality to another dimension.
void SimpleEstimator::countPaths(std::vector<std::pair<std::vector<std::pair<std::vector<uint32_t>, uint32_t>>, uint32_t>>& pathMatrix, uint32_t node) {
    // Get each transition from this node and call this function again.

    // Both adjacency matrices.
    std::vector<std::vector<std::pair<uint32_t,uint32_t>>>* adj = &(*graph).adj;
    std::vector<std::vector<std::pair<uint32_t,uint32_t>>>* r_adj = &(*graph).reverse_adj;

    // Get the transitions from both.
    std::vector<std::pair<uint32_t,uint32_t>>* ts = &(*adj)[node];
    std::vector<std::pair<uint32_t,uint32_t>>* r_ts = &(*r_adj)[node];

    // Go through all outgoing transitions from this node.
    for ( const auto &t : *ts ) {
        uint32_t label = t.first;
        uint32_t node = t.second;
        countPaths(pathMatrix[label].first, node);
        pathMatrix[label].second++;
    }

    // Go through all incoming transitions from this node.
    for ( const auto &t : *r_ts ) {
        uint32_t label = t.first;
        uint32_t node = t.second;
        countPaths(pathMatrix[L+label].first, node);
        pathMatrix[L+label].second++;
    }
}

// Function that takes a vector of vectors. It goes through all connecting nodes and either:
// - Calls itself again, with the corresponding vector of the transition, to go through the other nodes' nodes.
// - Calls another variant of itself that takes a vector of ints that actually increases the counts.
// (does this automatically based on data type that is passed along)
// Copy paste this function and add another vector layer to expand functionality to another dimension.
void SimpleEstimator::countPaths(std::vector<std::pair<std::vector<uint32_t>, uint32_t>>& pathMatrix, uint32_t node) {
    // Get each transition from this node and call this function again.

    // Both adjacency matrices.
    std::vector<std::vector<std::pair<uint32_t,uint32_t>>>* adj = &(*graph).adj;
    std::vector<std::vector<std::pair<uint32_t,uint32_t>>>* r_adj = &(*graph).reverse_adj;

    // Get the transitions from both.
    std::vector<std::pair<uint32_t,uint32_t>>* ts = &(*adj)[node];
    std::vector<std::pair<uint32_t,uint32_t>>* r_ts = &(*r_adj)[node];

    // Go through all outgoing transitions from this node.
    for ( const auto &t : *ts ) {
        uint32_t label = t.first;
        uint32_t node = t.second;
        countPaths(pathMatrix[label].first, node);
        pathMatrix[label].second++;
    }

    // Go through all incoming transitions from this node.
    for ( const auto &t : *r_ts ) {
        uint32_t label = t.first;
        uint32_t node = t.second;
        countPaths(pathMatrix[L+label].first, node);
        pathMatrix[L+label].second++;
    }
}

void SimpleEstimator::countPaths(std::vector<uint32_t>& pathVector, uint32_t node) {

    // Both adjacency matrices.
    std::vector<std::vector<std::pair<uint32_t,uint32_t>>>* adj = &(*graph).adj;
    std::vector<std::vector<std::pair<uint32_t,uint32_t>>>* r_adj = &(*graph).reverse_adj;

    // Get the transitions from both.
    std::vector<std::pair<uint32_t,uint32_t>>* ts = &(*adj)[node];
    std::vector<std::pair<uint32_t,uint32_t>>* r_ts = &(*r_adj)[node];

    // Go through all outgoing transitions from this node.
    for ( const auto &t : *ts ) {
        uint32_t label = t.first;
        pathVector[label]++;
    }

    // Go through all incoming transitions from this node.
    for ( const auto &t : *r_ts ) {
        uint32_t label = t.first;
        pathVector[L+label]++;
    }
}

cardStat SimpleEstimator::estimate(RPQTree *q) {
    if (estimateMethod == 0) {
        return estimateFirst(q);
    } else {
        return estimateBruteForce(q);
    }
}

cardStat SimpleEstimator::estimateFirst(RPQTree *q) {

    // Counts of potential start nodes and end nodes for this query.
    uint32_t potStartNodeCount, potEndNodeCount;
    // Probabilities for: the whole path, the path without start transition, without end transition.
    float pathProb, startPathProb, endPathProb, firstLabelProb, lastLabelProb;
    // Statistics to be reported at the end.
    uint32_t paths, startNodes, endNodes;

    auto leftmost = q;
    auto rightmost = q;
    while ( !leftmost->isLeaf() ) { leftmost = leftmost->left; }
    while ( !rightmost->isLeaf() ) { rightmost = rightmost->right; }
    std::string firstLabel = leftmost->data;
    std::string lastLabel = rightmost->data;
    uint32_t firstLabelInt = std::stoi(firstLabel);
    uint32_t lastLabelInt = std::stoi(lastLabel);

    if ( firstLabel[1] == '+' ) {
        potStartNodeCount = nodesWithOutLabel[firstLabelInt];
    } else {
        potStartNodeCount = nodesWithInLabel[firstLabelInt];
    }

    if ( lastLabel[1] == '+') {
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
    paths = (int)(pathProb*((float)N));
    startNodes = (int)(startPathProb*((float)potStartNodeCount));
    endNodes = (int)(endPathProb*((float)potEndNodeCount));

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
        int rootLabelInt = std::stoi(rootLabel);
        if (rootLabel[1] == '+') {
            query.push_back(rootLabelInt);
        } else {
            query.push_back(rootLabelInt + L);
        }
    }

    convertQuery(q->right, query);
}

void SimpleEstimator::prepareBruteForce()
{
    summary.resize((*graph).getNoVertices());

    std::cout << (*graph).getNoEdges() << std::endl;
    std::cout << (*graph).adj.size() << std::endl;
    std::cout << (*graph).getNoVertices() << std::endl;
    std::cout << (*graph).getNoLabels() << std::endl;
    std::cout << summary.size() << std::endl;

    for (size_t node = 1; node < (*graph).adj.size(); node++)
    {
        summary[node].resize(2);
        summary[node][0].insert(summary[node][0].end(),
                                (*graph).adj[node].begin(),
                                (*graph).adj[node].end());
    }


    for (size_t node = 1; node < (*graph).reverse_adj.size(); node++)
    {
        summary[node][1].insert(summary[node][1].end(), (*graph).reverse_adj[node].begin(), (*graph).reverse_adj[node].end());
    }
}

cardStat SimpleEstimator::estimateBruteForce(RPQTree *q)
{
    this->a_start_vertices = 0;
    this->unique_end_vertices.clear();
    int total = 0;
    auto query = treeToString(q);

    std::stringstream ss(query);
    std::string item;
    std::vector<std::string> path;

    while (std::getline(ss, item, '/'))
    {
        path.push_back(item);
    }

    if (q != nullptr) {
        for (size_t node = 1; node < summary.size(); node++) {
            total += subestimateBruteForce(path, node, true);
        }
    }

    return cardStat {this->a_start_vertices, total, this->unique_end_vertices.size()};
}

int SimpleEstimator::subestimateBruteForce(std::vector<std::string> path, int node, bool calculate_start_vertices)
{
    int total = 0;

    if (path.size() == 0)
    {
        this->unique_end_vertices.insert(node);
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
        if (edge.first == std::atoi(&path[0][0]))
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

int SimpleEstimator::estimateBruteForceStart(RPQTree *q)
{
    return 0;
}

int SimpleEstimator::estimateBruteForceEnd(RPQTree *q)
{
    return 0;
}

std::string SimpleEstimator::treeToString(RPQTree *q)
{
    std::string result;
    std::string l, r;

    if (q->right != nullptr)
    {
        r = treeToString(q->right);
        result = r;
    }

    result = q->data + result;

    if (q->left != nullptr)
    {
        l = treeToString(q->left);
        result = l + result;
    }

    return result;
}
