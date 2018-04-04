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

    for(uint32_t source = 0; source < g->getNoVertices(); source++) {
        if(g->begin(source) != g->end(source)) stats.noOut++;
    }

    stats.noPaths = g->getNoDistinctEdges();

    for(uint32_t target = 0; target < g->getNoVertices(); target++) {
        if(g->rbegin(target) != g->rend(target)) stats.noIn++;
    }

    return stats;
}

std::shared_ptr<SimpleGraph> SimpleEvaluator::project(uint32_t projectLabel, bool inverse, std::shared_ptr<SimpleGraph> &in) {

    auto out = std::make_shared<SimpleGraph>(in->getNoVertices());
    out->setNoLabels(in->getNoLabels());

    if(!inverse) {
        // going forward
        for(uint32_t source = 0; source < in->getNoVertices(); source++) {
            for (auto labelTarget = in->begin(source); labelTarget != in->end(source); labelTarget++) {

                auto target = labelTarget->first.second;
                auto labels = labelTarget->second;

                if (std::find(labels.begin(), labels.end(), projectLabel) != labels.end())
                    out->addEdge(source, target, projectLabel);
            }
        }
    } else {
        // going backward
        for(uint32_t source = 0; source < in->getNoVertices(); source++) {
            for (auto labelTarget = in->rbegin(source); labelTarget != in->rend(source); labelTarget++) {

                auto target = labelTarget->first.second;
                auto labels = labelTarget->second;

                if (std::find(labels.begin(), labels.end(), projectLabel) != labels.end())
                    out->addEdge(source, target, projectLabel);
            }
        }
    }

    return out;
}

std::shared_ptr<SimpleGraph> SimpleEvaluator::join(std::shared_ptr<SimpleGraph> &left, std::shared_ptr<SimpleGraph> &right) {

    auto out = std::make_shared<SimpleGraph>(left->getNoVertices());
    out->setNoLabels(1);

    for(uint32_t leftSource = 0; leftSource < left->getNoVertices(); leftSource++) {
        for (auto labelTarget = left->begin(leftSource); labelTarget != left->end(leftSource); labelTarget++) {

            uint32_t leftTarget = labelTarget->first.second;
            // try to join the left target with right source
            for (auto rightLabelTarget = right->begin(leftTarget); rightLabelTarget != right->end(leftTarget); rightLabelTarget++) {

                auto rightTarget = rightLabelTarget->first.second;
                out->addEdge(leftSource, rightTarget, 0);

            }
        }
    }

    return out;
}

std::shared_ptr<SimpleGraph> SimpleEvaluator::evaluate_aux(RPQTree *q) {

    // evaluate according to the AST bottom-up

    if(q->isLeaf()) {
        // project out the label in the AST
        std::regex directLabel (R"((\d+)\+)");
        std::regex inverseLabel (R"((\d+)\-)");

        std::smatch matches;

        uint32_t label;
        bool inverse;

        if(std::regex_search(q->data, matches, directLabel)) {
            label = (uint32_t) std::stoul(matches[1]);
            inverse = false;
        } else if(std::regex_search(q->data, matches, inverseLabel)) {
            label = (uint32_t) std::stoul(matches[1]);
            inverse = true;
        } else {
            std::cerr << "Label parsing failed!" << std::endl;
            return nullptr;
        }

        return SimpleEvaluator::project(label, inverse, graph);
    }

    if(q->isConcat()) {

        // evaluate the children
        auto leftGraph = SimpleEvaluator::evaluate_aux(q->left);
        auto rightGraph = SimpleEvaluator::evaluate_aux(q->right);

        // join left with right
        return SimpleEvaluator::join(leftGraph, rightGraph);

    }

    return nullptr;
}

cardStat SimpleEvaluator::evaluate(RPQTree *query) {
    auto res = evaluate_aux(query);
    return SimpleEvaluator::computeStats(res);
}