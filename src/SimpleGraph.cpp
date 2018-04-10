//
// Created by Nikolay Yakovets on 2018-01-31.
//

#include "SimpleGraph.h"

SimpleGraph::SimpleGraph(uint32_t n_V, uint32_t n_L)   {
    setNoVertices(n_V);
    setNoLabels(n_L);
}

uint32_t SimpleGraph::getNoVertices() const {
    return V;
}

void SimpleGraph::setNoVertices(uint32_t n) {
    V = n;
    adj.resize(n);
    reverse_adj.resize(n);
}

uint32_t SimpleGraph::getNoEdges() const {
    uint32_t sum = 0;
    for (const auto & l : adj)
        sum += l.size();
    return sum;
}

uint32_t SimpleGraph::getNoDistinctEdges() const {

    uint32_t sum = 0;

    for (auto sourceVec : adj) {

        std::sort(sourceVec.begin(), sourceVec.end(), [](const std::pair<uint32_t,uint32_t> &a, const std::pair<uint32_t,uint32_t> &b){
            if (a.second == b.second)
                return a.first < b.first;
            return a.second < b.second;
        });

        uint32_t prevTarget = 0;
        uint32_t prevLabel = 0;
        bool first = true;

        for (const auto &labelTgtPair : sourceVec) {
            if (first || !(prevTarget == labelTgtPair.second && prevLabel == labelTgtPair.first)) {
                first = false;
                sum++;
                prevTarget = labelTgtPair.second;
                prevLabel = labelTgtPair.first;
            }
        }
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
    adj.at(from).emplace_back(edgeLabel, to);
    reverse_adj.at(to).emplace_back(edgeLabel, from);
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

InterGraph::InterGraph(uint32_t n) : Graph(), V(n) {
    edges.resize(0);
}

uint32_t InterGraph::getNoVertices() const {
    return V;
}

uint32_t InterGraph::getNoEdges() const {
    return edges.size();
}

uint32_t InterGraph::getNoDistinctEdges() const {

    auto& v = const_cast<std::vector<std::pair<uint32_t, uint32_t>>&>(edges);
    std::sort(v.begin(), v.end(), [](const std::pair<uint32_t,uint32_t> &a, const std::pair<uint32_t,uint32_t> &b){
        if(a.first == b.first)
            return a.second < b.second;
        return a.first < b.first;
    });

    uint32_t sum = 0;

    uint32_t prevSrc = 0;
    uint32_t prevTgt = 0;
    bool first = true;
    for (auto edge : edges) {

        if(first || !(prevSrc == edge.first && prevTgt == edge.second)) {
            first = false;
            sum++;
            prevSrc = edge.first;
            prevTgt = edge.second;
        }
    }

    return sum;
}

uint32_t InterGraph::getNoLabels() const {
    return 1;
}

void InterGraph::addEdge(uint32_t from, uint32_t to, uint32_t) {
    if(from >= V || to >= V)
        throw std::runtime_error(std::string("Edge data out of bounds: ") +
                                 "(" + std::to_string(from) + "," + std::to_string(to) + ")");

    auto e = std::make_pair(from, to);

//    if(std::find(edges.begin(), edges.end(), e) == edges.end())
        edges.push_back(e);
}

void InterGraph::readFromContiguousFile(const std::string &fileName) {
    throw std::runtime_error("Invalid operation");
}
