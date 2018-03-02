//
// Created by Nikolay Yakovets on 2018-02-01.
//

#include "SimpleGraph.h"
#include "SimpleEstimator.h"

// TODO, are the esimates definitely calculated correctly?
// TODO, create appropriate deconstructor

SimpleEstimator::SimpleEstimator(std::shared_ptr<SimpleGraph> &g){

    // works only with SimpleGraph
    graph = g;
}

void SimpleEstimator::prepare() {

    // Instead of just looking at single transitions, record number of occurences of multiple transitions in series (start with two).

    // Total number of nodes and number of different labels.
    N = (*graph).getNoVertices();
    L = (*graph).getNoLabels();

    // Different counts.
    // Total number of times a specific label occurs.
    std::vector<uint32_t> labels1;
    labels1.resize(L);

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

    // This can be left as-is (probably).
    // TODO, remove single transition counts.
    // =============================================================================================
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
            labels1[label]++;
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
    // =============================================================================================

    // ADAPT
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

    // Outdated.
    // ===============================================================================================
    // Calculate the transition label probabilities.
    /*for ( int i = 0; i < labels1.size(); i++) {
        labelProbs[i] = ((float)labels1[i])/((float)N);
        //std::cout << std::endl << "Label " << i << ": " << "Seen " << labels1[i] << " times, Prob~: " << labelProbs[i];
    }*/
    // ================================================================================================

}

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
    //std::cout << std::endl << "First label: " << firstLabel;
    //std::cout << std::endl << "Last label: " << lastLabel;

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

    // TODO
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

    /*std::cout << "Converted QUERY: ";
    for (int i = 0; i < query.size(); ++i) {
        std::cout << query[i];
    }
    std::cout << std::endl;*/

    // Outdated.
    // ======================================================================================================
    // Go through whole tree and multiply the probs of all transitions that are encountered recursively.
    /*pathProb = calcProb(q);
    // To calculate probs of remaining start and end path, just divide by start and end transition probs.
    firstLabelProb = labelProbs[firstLabelInt];
    lastLabelProb = labelProbs[lastLabelInt];
    startPathProb = pathProb / firstLabelProb;
    endPathProb = pathProb / lastLabelProb;*/
    // ========================================================================================================

    // To obtain final metrics, multiply path prob by N and start/end by the respective potNodeCounts.
    paths = (int)(pathProb*((float)N));
    startNodes = (int)(startPathProb*((float)potStartNodeCount));
    endNodes = (int)(endPathProb*((float)potEndNodeCount));

    // TODO, (not necessary) fix the start and end node estimations

    //std::cout << std::endl << potStartNodeCount;
    //std::cout << std::endl << potEndNodeCount;

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


    /*int qs = query.size();
    float prob = 1;
    for (int i = 0; i < qs; i = i + 3) {
        if (qs - i >= 3) {
            uint32_t t1 = query[i];
            uint32_t t2 = query[i+1];
            uint32_t t3 = query[i+2];
            prob *= pathProbs3[t1].first[t2].first[t3];
        } else if (qs - i >= 2) {
            uint32_t t1 = query[i];
            uint32_t t2 = query[i+1];
            prob *= pathProbs3[t1].first[t2].second;
        } else {
            uint32_t t1 = query[i];
            prob *= pathProbs3[t1].second;
        }
    }
    return prob;*/
}

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

float SimpleEstimator::calcProb(RPQTree *q) {
    // Get prob of root node, multiply by prob of left and right subtrees if they exist.
    float prob;

    std::string rootLabel = q->data;
    if ( rootLabel == "/" ) {
        prob = 1;
    } else {
        int rootLabelInt = std::stoi(rootLabel);
        prob = labelProbs[rootLabelInt];
    }

    if ( q->left != nullptr) {
        prob *= calcProb(q->left);
    }
    if ( q->right != nullptr ) {
        prob *= calcProb(q->right);
    }

    return prob;
}