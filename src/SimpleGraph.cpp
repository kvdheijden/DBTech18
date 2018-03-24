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
SimpleGraph::SimpleGraph(const std::string& file) {
    this->readFromContiguousFile(file);
}

SimpleGraph::SimpleGraph(uint32_t n_V, uint32_t n_L) {
    setNoVertices(n_V);
    this->L = n_L;
}

SimpleGraph::SimpleGraph(uint32_t n_L) : SimpleGraph(0, n_L) {}
SimpleGraph::SimpleGraph() : SimpleGraph(0) {}

uint32_t SimpleGraph::getNoVertices() const {
    return this->V.size();
}

void SimpleGraph::setNoVertices(uint32_t v) {
    this->V.clear();
    for(uint32_t i = 0; i < v; i++) {
        this->V.emplace_back(i);
    }
}

uint32_t SimpleGraph::getNoEdges() const {
    return this->E.size();
}

uint32_t SimpleGraph::getNoLabels() const {
    return L;
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
    if(from >= getNoVertices() || to >= getNoVertices() || edgeLabel >= getNoLabels())
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
    uint32_t n_E, n_V;
    if(std::regex_search(line, matches, headerPat)) {
        n_V = (uint32_t) std::stoul(matches[1]);
        n_E = (uint32_t) std::stoul(matches[2]);
        this->L = ((uint32_t) std::stoul(matches[3]));

        this->setNoVertices(n_V);
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

    if(this->E.size() != n_E)
        throw std::runtime_error("Invalid number of edges!");
    if(this->V.size() != n_V)
        throw std::runtime_error("Invalid number of vertices");
}

SimpleVertex &SimpleGraph::getVertex(uint32_t i) {
    if(i >= getNoVertices())
        throw std::runtime_error(std::string("Vertex data out of bound: (") + std::to_string(i) + ")");
    return this->V[i];
}
