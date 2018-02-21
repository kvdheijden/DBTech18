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

    // Total number of nodes and number of different labels.
    N = (*graph).getNoVertices();
    L = (*graph).getNoLabels();

    // Different counts.
    // Total number of times a specific label occurs.
    std::vector<uint32_t> labels;
    labels.resize(L);
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
        for ( int i; i < seenLabelForNode.size(); i++ ) {
            seenLabelForNode[i] = false;
        }
        // Go through all transitions from this node.
        for ( const auto &t : n ) {
            uint32_t label = t.first;
            labels[label]++;
            if ( !seenLabelForNode[label] ) {
                nodesWithOutLabel[label]++;
            }
        }
    }

    // Go through all nodes and count the number of nodes with specific incoming transitions.
    auto reverse_adj = &(*graph).reverse_adj;
    for ( const auto &n : *reverse_adj ) {
        // Reset flags for which labels have already been seen for this node.
        for ( int i; i < seenLabelForNode.size(); i++ ) {
            seenLabelForNode[i] = false;
        }
        // Go through all transitions to this node.
        for ( const auto &t : n ) {
            uint32_t label = t.first;
            if ( !seenLabelForNode[label] ) {
                nodesWithInLabel[label]++;
            }
        }
    }

    // Calculate the transition label probabilities.
    for ( int i = 0; i < labels.size(); i++) {
        labelProbs[i] = ((float)labels[i])/((float)N);
        std::cout << std::endl << "Label " << i << ": " << "Seen " << labels[i] << " times, Prob~: " << labelProbs[i];
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
    std::cout << std::endl << "First label: " << firstLabel;
    std::cout << std::endl << "Last label: " << lastLabel;

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

    // Go through whole tree and multiply the probs of all transitions that are encountered recursively.
    pathProb = calcProb(q);
    // To calculate probs of remaining start and end path, just divide by start and end transition probs.
    firstLabelProb = labelProbs[firstLabelInt];
    lastLabelProb = labelProbs[lastLabelInt];
    startPathProb = pathProb / firstLabelProb;
    endPathProb = pathProb / lastLabelProb;

    // To obtain final metrics, multiply path prob by N and start/end by the respective potNodeCounts.
    paths = (int)(pathProb*((float)N));
    startNodes = (int)(startPathProb*((float)potStartNodeCount));
    endNodes = (int)(endPathProb*((float)potEndNodeCount));

    std::cout << std::endl << potStartNodeCount;
    std::cout << std::endl << potEndNodeCount;

    return cardStat {startNodes, paths, endNodes};
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