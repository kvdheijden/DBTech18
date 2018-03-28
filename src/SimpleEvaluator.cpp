//
// Created by Nikolay Yakovets on 2018-02-02.
//

#include <chrono>
//#include <unistd.h>
#include "SimpleEstimator.h"
#include "SimpleEvaluator.h"

SimpleEvaluator::SimpleEvaluator(std::shared_ptr<SimpleGraph> &g) {

    // works only with SimpleGraph
    graph = g;

    // estimator not attached by default
    est = nullptr;
}

void SimpleEvaluator::attachEstimator(std::shared_ptr<SimpleEstimator> &e) {
    est = e;
}

void SimpleEvaluator::prepare() {

    // if attached, prepare the estimator
    if(est != nullptr) est->prepare();

    // prepare other things here.., if necessary
    // TODO: caching
}

cardStat SimpleEvaluator::computeStats(std::shared_ptr<SimpleGraph> &g) {

    cardStat stats {};

    for(uint32_t source = 0; source < g->getNoVertices(); source++) {
        if(g->getVertex(source).outDegree() != 0) stats.noOut++;
    }

    stats.noPaths = g->getNoDistinctEdges();

    for(uint32_t target = 0; target < g->getNoVertices(); target++) {
        if(g->getVertex(target).inDegree() != 0) stats.noIn++;
    }

    return stats;
}

std::shared_ptr<SimpleGraph> SimpleEvaluator::project(uint32_t projectLabel, bool inverse, std::shared_ptr<SimpleGraph> &in) {

    auto out = std::make_shared<SimpleGraph>(in->getNoVertices(), in->getNoLabels());

    if(!inverse) {
        // going forward
        for(uint32_t source = 0; source < in->getNoVertices(); source++) {
            for (const SimpleEdge *labelTarget : in->getVertex(source).outgoing()) {
                if (labelTarget->label == projectLabel)
                    out->addEdge(source, labelTarget->target.label, projectLabel);
            }
        }

        return out;
    } else {
        // going backward
        for(uint32_t source = 0; source < in->getNoVertices(); source++) {
            for (const SimpleEdge* labelTarget : in->getVertex(source).incoming()) {
                if (labelTarget->label == projectLabel)
                    out->addEdge(source, labelTarget->source.label, projectLabel);
            }
        }

        return out;
    }
}

std::shared_ptr<SimpleGraph> SimpleEvaluator::join(std::shared_ptr<SimpleGraph> &left, std::shared_ptr<SimpleGraph> &right) {

    auto out = std::make_shared<SimpleGraph>(left->getNoVertices(), 1);

    for(uint32_t leftSource = 0; leftSource < left->getNoVertices(); leftSource++) {
        for (const SimpleEdge* labelTarget : left->getVertex(leftSource).outgoing()) {
            // try to join the left target with right source
            for (const SimpleEdge* rightLabelTarget : right->getVertex(labelTarget->target.label).outgoing()) {
                out->addEdge(leftSource, rightLabelTarget->target.label, 0);
            }
        }
    }

    return out;
}

std::shared_ptr<SimpleGraph> SimpleEvaluator::evaluate_aux(RPQTree *q) {

    // evaluate according to the AST bottom-up
    if(q->isLeaf()) {
        const char *c_str = q->data.c_str();
        char *end;
        uint32_t label = std::strtoul(c_str, &end, 10);

        return project(label, *end == '-', graph);
    } else if (q->isUnary()) {
        return evaluate_aux(q->left);
    } else if (q->isBinary()) {
        std::shared_ptr<SimpleGraph> left = evaluate_aux(q->left);
        std::shared_ptr<SimpleGraph> right = evaluate_aux(q->right);

        return join(left, right);
    }

    throw std::runtime_error("Invalid RPQTree for evaluation");
}

/* // Could be useful later on...
uint32_t SimpleEvaluator::convertLabelToInt(std::string label) {
    unsigned int rootLabelInt = (unsigned int)std::stoul(label);
    if (label[label.size()-1] == '+') {
        return rootLabelInt;
    } else {
        return rootLabelInt + est->L;
    }
}
 */

std::string SimpleEvaluator::convertIntToLabel(uint32_t i) {
    if (i < est->L) {
        std::string str = std::to_string(i) + "+";
        return str;
    } else {
        std::string str = std::to_string(i-est->L) + "-";
        return str;
    }
}

// Construct an AST where the total plan is estimated to be most optimal.
RPQTree* SimpleEvaluator::convertEfficientAST(RPQTree *q) {
    // First convert the RPQTree to a vector to make life a lot easier.
    std::vector<uint32_t> query;
    est->convertQuery(q, query);

    // Initialise unordered (hash)map.
    std::unordered_map<std::string, uint32_t> estimationCache;

    uint32_t totalCost = 0;
    // Generate the most efficient AST using a recursive algorithm.
    return generateEfficientAST(query, totalCost, estimationCache);
}

RPQTree* SimpleEvaluator::generateEfficientAST(std::vector<uint32_t> &query, uint32_t &totalCost, std::unordered_map<std::string, uint32_t> &ec) {
    // If the query has size one, add no cost and return a leaf with this transition.
    // (Could also add the number of transitions to the cost, but since this is a constant effort that is always the same, regardless of the plan, we might as well not do it.
    if (query.size() == 1) {
        std::string data = convertIntToLabel(query[0]);
        return new RPQTree(data, nullptr, nullptr);
    }

    // Try out all different possible plans using estimation.
    // Do a join at every possible index, recursively generate most efficient plans for each side.
    uint32_t bestCost = INT32_MAX;
    RPQTree* bestPlan;

    for (int i = 1; i < query.size(); ++i) {
        uint32_t cost = 0;

        // In a join, estimate the cost of the join by multiplying the estimate number of paths of the two subqueries.
        // Add to this the lowest cost of the subqueries to get the total cost of this plan.
        std::vector<uint32_t> subQuery1(query.begin(), query.begin() + i);
        std::vector<uint32_t> subQuery2(query.begin() + i, query.begin() + query.size());

        // Get the best plans for the subqueries and add their costs to cost.
        RPQTree *subQueryTree1 = generateEfficientAST(subQuery1, cost, ec);
        if (cost >= bestCost) { continue; } // If cost is already higher, no need to explore further.
        RPQTree *subQueryTree2 = generateEfficientAST(subQuery2, cost, ec);
        if (cost >= bestCost) { continue; } // If cost is already higher, no need to explore further.

        int estimate1;
        int estimate2;
        // Get estimated cost of join.
        if (ec.find(vecToString(subQuery1)) != ec.end()) { // Check if in cache.
                estimate1 = ec[vecToString(subQuery1)];
            } else {
            estimate1 = est->estimate(subQueryTree1).noPaths;
            //usleep(1000); // Artificial estimation time increase to check caching effect.
            ec[vecToString(subQuery1)] = estimate1;
        }
        if (ec.find(vecToString(subQuery2)) != ec.end()) { // Check if in cache.
            estimate2 = ec[vecToString(subQuery2)];
        } else {
            estimate2 = est->estimate(subQueryTree2).noPaths;
            //usleep(1000); // Artificial estimation time increase to check caching effect.
            ec[vecToString(subQuery2)] = estimate2;
        }
        cost += estimate1 * estimate2;

        // Keep track of the plan with the lowest score so far.
        if (cost <= bestCost) {
            bestCost = cost;
            std::string slash = "/";
            bestPlan = new RPQTree(slash, subQueryTree1, subQueryTree2);
        }
    }

    totalCost += bestCost;
    return bestPlan;
}

std::string SimpleEvaluator::vecToString(std::vector<uint32_t> vec) {
    std::ostringstream oss;

    if (!vec.empty())
    {
        // Convert all but the last element to avoid a trailing ","
        std::copy(vec.begin(), vec.end()-1,
                  std::ostream_iterator<int>(oss, ","));

        // Now add the last element with no delimiter
        oss << vec.back();
    }

    return oss.str();
}

cardStat SimpleEvaluator::evaluate(RPQTree *query) {

    // Re-order the AST to a more efficient plan.
//    auto start = std::chrono::steady_clock::now();
//    RPQTree *queryEff = convertEfficientAST(query);
//    auto end = std::chrono::steady_clock::now();
//    std::cout << "Time to plan: " << std::chrono::duration<double, std::milli>(end - start).count() << " ms" << std::endl;

    //RPQTree *queryEff = query;

    //std::cout << std::endl << "Converted parsed query tree: ";
    //queryEff->print();

    std::shared_ptr<SimpleGraph> res = evaluate_aux(query);
    return computeStats(res);
}