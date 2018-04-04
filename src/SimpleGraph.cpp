//
// Created by Nikolay Yakovets on 2018-01-31.
//

#include <set>
#include "SimpleGraph.h"

SimpleGraph::SimpleGraph(uint32_t n)   {
    setNoVertices(n);
}

uint32_t SimpleGraph::getNoVertices() const {
    return V;
}

void SimpleGraph::setNoVertices(uint32_t n) {
    V = n;
}

uint32_t SimpleGraph::getNoEdges() const {
    uint32_t sum = 0;
    for (const auto & l : adj)
        sum += l.second.size();
    return sum;
}

uint32_t SimpleGraph::getNoDistinctEdges() const {

    uint32_t sum = 0;

    for (const auto &sourceVec : adj) {
        std::set<uint32_t> s(sourceVec.second.begin(), sourceVec.second.end());
        sum += s.size();
    }

    return sum;
}

uint32_t SimpleGraph::getNoLabels() const {
    return L;
}

void SimpleGraph::setNoLabels(uint32_t noLabels) {
    L = noLabels;
}

void SimpleGraph::addEdge(uint32_t from, uint32_t to, uint32_t edgeLabel) {
    if(from >= V || to >= V || edgeLabel >= L)
        throw std::runtime_error(std::string("Edge data out of bounds: ") +
                                         "(" + std::to_string(from) + "," + std::to_string(to) + "," +
                                         std::to_string(edgeLabel) + ")");

    adj[std::make_pair(from, to)].push_back(edgeLabel);
    r_adj[std::make_pair(to, from)].push_back(edgeLabel);
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
        uint32_t noNodes = (uint32_t) std::stoul(matches[1]);
        uint32_t noLabels = (uint32_t) std::stoul(matches[3]);

        setNoVertices(noNodes);
        setNoLabels(noLabels);
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

}

SimpleGraph::adjacency_matrix::const_iterator SimpleGraph::begin(uint32_t n) const {
    static constexpr uint32_t min = std::numeric_limits<uint32_t>::min();
    return adj.lower_bound({n, min});
}

SimpleGraph::adjacency_matrix::const_iterator SimpleGraph::rbegin(uint32_t n) const {
    static constexpr uint32_t min = std::numeric_limits<uint32_t>::min();
    return r_adj.lower_bound({n, min});
}

SimpleGraph::adjacency_matrix::const_iterator SimpleGraph::end(uint32_t n) const {
    static constexpr uint32_t max = std::numeric_limits<uint32_t>::max();
    return adj.upper_bound({n, max});
}

SimpleGraph::adjacency_matrix::const_iterator SimpleGraph::rend(uint32_t n) const {
    static constexpr uint32_t max = std::numeric_limits<uint32_t>::max();
    return r_adj.upper_bound({n, max});
}
