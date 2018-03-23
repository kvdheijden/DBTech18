//
// Created by Nikolay Yakovets on 2018-01-31.
//

#include "SimpleGraph.h"

/// SimpleEdge
SimpleEdge::SimpleEdge(uint32_t label, const SimpleVertex& subject, const SimpleVertex& object)
        : label(label), source(subject), target(object) {}

/// SimpleVertex
static bool edgeSort(const SimpleEdge *a, const SimpleEdge *b) {
    if(a->target == b->target) {
        return a->label < b->label;
    }
    return a->target < b->target;
}

SimpleVertex::SimpleVertex(uint32_t l) : adj(edgeSort), r_adj(edgeSort), label(l) {}

bool SimpleVertex::operator<(const SimpleVertex &other) const {
    return label < other.label;
}

bool SimpleVertex::operator==(const SimpleVertex &other) const {
    return label == other.label;
}

const std::set<const SimpleEdge *, edge_sort_fcn> &SimpleVertex::outgoing() const {
    return adj;
}

const std::set<const SimpleEdge *, edge_sort_fcn> &SimpleVertex::incoming() const {
    return r_adj;
}

void SimpleVertex::insert_outgoing(const SimpleEdge& e) {
    this->adj.insert(&e);
}

void SimpleVertex::insert_incoming(const SimpleEdge& e) {
    this->r_adj.insert(&e);
}

/// SimpleGraph
SimpleGraph::SimpleGraph() : n_V(0), n_L(0), n_E(0) {};

uint32_t SimpleGraph::getNoVertices() const {
    return n_V;
}

void SimpleGraph::setNoVertices(uint32_t v) {
    this->n_V = v;
    this->V.clear();
    for(uint32_t i = 0; i < v; i++) {
        this->V.emplace_back(i);
    }
}

uint32_t SimpleGraph::getNoEdges() const {
    return n_E;
}

uint32_t SimpleGraph::getNoLabels() const {
    return n_L;
}

void SimpleGraph::setNoLabels(uint32_t l) {
    this->n_L = l;
}

uint32_t SimpleGraph::getNoDistinctEdges() const {

    uint32_t sum = 0;

    for (const SimpleVertex& sourceVec : V) {

        // std::sort not needed since std::set is sorted by default.

        const SimpleVertex* prevTarget = nullptr;
        uint32_t prevLabel = 0;

        for (const SimpleEdge* edge : sourceVec.outgoing()) {
            if (!(prevTarget == &edge->target && prevLabel == edge->label)) {
                sum++;
                prevTarget = &edge->target;
                prevLabel = edge->label;
            }
        }
    }

    return sum;
}

void SimpleGraph::addEdge(uint32_t from, uint32_t to, uint32_t edgeLabel) {
    if(from >= n_V || to >= n_V || edgeLabel >= n_L)
        throw std::runtime_error(std::string("Edge data out of bounds: ") +
                                         "(" + std::to_string(from) + "," + std::to_string(to) + "," +
                                         std::to_string(edgeLabel) + ")");

    SimpleVertex& subject = this->V[from];
    SimpleVertex& object = this->V[to];

    this->E.emplace_back(edgeLabel, subject, object);
    const SimpleEdge& predicate = this->E.back();

    subject.insert_outgoing(predicate);
    object.insert_incoming(predicate);
}

void SimpleGraph::readFromContiguousFile(const std::string &fileName) {

    std::string line;
    std::ifstream graphFile { fileName };

    std::regex edgePat (R"((\d+)\s(\d+)\s(\d+)\s\.)"); // subject predicate object .
    std::regex headerPat (R"((\d+),(\d+),(\d+))"); // noNodes,noEdges,noLabels

    // parse the header (1st line)
    std::getline(graphFile, line);
    std::smatch matches;
    if(std::regex_search(line, matches, headerPat)) {
        this->setNoVertices((uint32_t) std::stoul(matches[1]));
        this->n_E = (uint32_t) std::stoul(matches[2]);
        this->setNoLabels((uint32_t) std::stoul(matches[3]));

        this->E.clear();
    } else {
        throw std::runtime_error(std::string("Invalid graph header!"));
    }

    // parse edge data
    while(std::getline(graphFile, line)) {

        if(std::regex_search(line, matches, edgePat)) {
            uint32_t subject = (uint32_t) std::stoul(matches[1]);
            uint32_t predicate = (uint32_t) std::stoul(matches[2]);
            uint32_t object = (uint32_t) std::stoul(matches[3]);

            addEdge(subject, object, predicate);
        }
    }

    graphFile.close();

    if(this->E.size() != this->n_E)
        throw std::runtime_error("Invalid number of edges!");
    if(this->V.size() != this->n_V)
        throw std::runtime_error("Invalid number of vertices");
}

SimpleVertex &SimpleGraph::getVertex(uint32_t i) {
    if(i >= this->n_V)
        throw std::runtime_error(std::string("Vertex data out of bound: (") + std::to_string(i) + ")");
    return this->V[i];
}
