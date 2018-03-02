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
    //prepareFirst();
    prepareBruteForce();
}

void SimpleEstimator::prepareFirst() {

    // Total number of nodes and number of different labels.
    N = (*graph).getNoVertices();
    L = (*graph).getNoLabels();

    // Different counts.
    // Total number of times a specific label occurs.
    std::vector<uint32_t> amountOfOccurrencesOfLabels;
    amountOfOccurrencesOfLabels.resize(L);
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
            amountOfOccurrencesOfLabels[label]++;
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
    for ( int i = 0; i < amountOfOccurrencesOfLabels.size(); i++) {
        labelProbs[i] = ((float)amountOfOccurrencesOfLabels[i])/((float)N);
        std::cout << std::endl << "Label " << i << ": " << "Seen " << amountOfOccurrencesOfLabels[i] << " times, Prob~: " << labelProbs[i];
    }

}

cardStat SimpleEstimator::estimate(RPQTree *q) {
    //return estimateFirst(q);
    return estimateBruteForce(q);
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

    std::cout << "props" << firstLabelProb << "," << lastLabelProb << std::endl;
    std::cout << "pots" << potStartNodeCount << "," << potEndNodeCount << std::endl;

    startPathProb = pathProb / firstLabelProb;
    endPathProb = pathProb / lastLabelProb;

    // To obtain final metrics, multiply path prob by N and start/end by the respective potNodeCounts.
    paths = (int)(pathProb*((float)N));
    startNodes = (int)(startPathProb*((float)potStartNodeCount));
    endNodes = (int)(endPathProb*((float)potEndNodeCount));

    std::cout << std::endl << potStartNodeCount;
    std::cout << std::endl << potEndNodeCount;
    std::cout << std::endl << "start" << startNodes;
    std::cout << std::endl << "end" << endNodes;
    std::cout << std::endl << "pathProb" << pathProb;
    std::cout << std::endl << "start" << startPathProb;
    std::cout << std::endl << "end" << endPathProb;
    std::cout << std::endl << "path" << pathProb;

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