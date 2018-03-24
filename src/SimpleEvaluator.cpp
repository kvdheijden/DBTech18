//
// Created by Nikolay Yakovets on 2018-02-02.
//

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

                auto label = labelTarget->label;
                auto target = labelTarget->target.label;

                if (label == projectLabel)
                    out->addEdge(source, target, label);
            }
        }
    } else {
        // going backward
        for(uint32_t source = 0; source < in->getNoVertices(); source++) {
            for (const SimpleEdge* labelTarget : in->getVertex(source).incoming()) {

                auto label = labelTarget->label;
                auto target = labelTarget->source.label;

                if (label == projectLabel)
                    out->addEdge(source, target, label);
            }
        }
    }

    return out;
}

std::shared_ptr<SimpleGraph> SimpleEvaluator::join(std::shared_ptr<SimpleGraph> &left, std::shared_ptr<SimpleGraph> &right) {

    auto out = std::make_shared<SimpleGraph>(left->getNoVertices(), 1);

    for(uint32_t leftSource = 0; leftSource < left->getNoVertices(); leftSource++) {
        for (const SimpleEdge* labelTarget : left->getVertex(leftSource).outgoing()) {

            uint32_t leftTarget = labelTarget->target.label;
            // try to join the left target with right source
            for (const SimpleEdge* rightLabelTarget : right->getVertex(leftTarget).outgoing()) {

                uint32_t rightTarget = rightLabelTarget->target.label;
                out->addEdge(leftSource, rightTarget, 0);
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

cardStat SimpleEvaluator::evaluate(RPQTree *query) {
    std::shared_ptr<SimpleGraph> res = evaluate_aux(query);
    return computeStats(res);
}