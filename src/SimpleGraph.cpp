//
// Created by Nikolay Yakovets on 2018-01-31.
//

#include "SimpleGraph.h"

SimpleGraph::SimpleGraph(uint32_t n_V, uint32_t n_L) {
    setNoVertices(n_V);
    setNoLabels(n_L);
    setNoEdges(0);
}

SimpleGraph::SimpleGraph(uint32_t n_L) : SimpleGraph(0, n_L) {}
SimpleGraph::SimpleGraph() : SimpleGraph(0) {}

uint32_t SimpleGraph::getNoVertices() const {
    return this->V;
}

void SimpleGraph::setNoVertices(uint32_t v) {
    this->V = v;
}

uint32_t SimpleGraph::getNoEdges() const {
    uint32_t sum = 0;
    for(const auto& v : adj) {
        sum += v.second.size();
    }
    return sum;
}

void SimpleGraph::setNoEdges(uint32_t e) {
    this->E = e;
}

uint32_t SimpleGraph::getNoLabels() const {
    return L;
}

void SimpleGraph::setNoLabels(uint32_t l) {
    this->L = l;
}

uint32_t SimpleGraph::getNoDistinctEdges() const {
    uint32_t sum = 0;

    for (const auto& edgeList : adj) {
        std::set<uint32_t> s(edgeList.second.begin(), edgeList.second.end());
        sum += s.size();
    }

    return sum;
}

void SimpleGraph::addEdge(uint32_t from, uint32_t to, uint32_t edgeLabel) {
    if(from >= getNoVertices() || to >= getNoVertices() || edgeLabel >= getNoLabels())
        throw std::runtime_error(std::string("Edge data out of bounds: ") +
                                         "(" + std::to_string(from) + "," + std::to_string(to) + "," +
                                         std::to_string(edgeLabel) + ")");

    adj[std::make_pair(from, to)].emplace_back(edgeLabel);
    r_adj[std::make_pair(to, from)].emplace_back(edgeLabel);
}

void SimpleGraph::readFromContiguousFile(const std::string &fileName) {

    std::string line;
    std::ifstream graphFile { fileName };

    std::regex edgePat (R"((\d+)\s(\d+)\s(\d+)\s\.)"); // subject predicate object .
    std::regex headerPat (R"((\d+),(\d+),(\d+))"); // noNodes,noEdges,noLabels

    // parse the header (1st line)
    std::getline(graphFile, line);
    std::smatch matches;
    uint32_t n_E, n_V, n_L;
    if(std::regex_search(line, matches, headerPat)) {
        n_V = (uint32_t) std::stoul(matches[1]);
        n_E = (uint32_t) std::stoul(matches[2]);
        n_L = ((uint32_t) std::stoul(matches[3]));

        this->setNoVertices(n_V);
        this->setNoEdges(n_E);
        this->setNoLabels(n_L);
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

    if(this->E != getNoEdges())
        throw std::runtime_error("Edges mismatch");
}

SimpleGraph::adjacency_matrix::const_iterator SimpleGraph::begin(uint32_t n) const {
    static constexpr int min = std::numeric_limits<uint32_t>::min();
    return adj.lower_bound({n, min});
}

SimpleGraph::adjacency_matrix::const_iterator SimpleGraph::end(uint32_t n) const {
    static constexpr int max = std::numeric_limits<uint32_t>::max();
    return adj.upper_bound({n, max});
}

SimpleGraph::adjacency_matrix::const_iterator SimpleGraph::rbegin(uint32_t n) const {
    static constexpr int min = std::numeric_limits<uint32_t>::min();
    return r_adj.lower_bound({n, min});
}

SimpleGraph::adjacency_matrix::const_iterator SimpleGraph::rend(uint32_t n) const {
    static constexpr int max = std::numeric_limits<uint32_t>::max();
    return r_adj.upper_bound({n, max});
}

