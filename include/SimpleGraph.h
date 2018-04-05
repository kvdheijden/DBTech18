//
// Created by Nikolay Yakovets on 2018-01-31.
//

#ifndef QS_SIMPLEGRAPH_H
#define QS_SIMPLEGRAPH_H

#include <algorithm>
#include <fstream>
#include <regex>
#include <vector>
#include <set>

#include "Graph.h"

class SimpleGraph : public Graph {
public:
    std::vector<std::vector<std::pair<uint32_t,uint32_t>>> adj;
    std::vector<std::vector<std::pair<uint32_t,uint32_t>>> reverse_adj; // vertex adjacency list
protected:
    uint32_t V;
    uint32_t L;

    void setNoVertices(uint32_t n);
    void setNoLabels(uint32_t n);

public:
    SimpleGraph() : SimpleGraph(0) {};
    explicit SimpleGraph(uint32_t n_V) : SimpleGraph(n_V, 0) {};
    SimpleGraph(uint32_t n_V, uint32_t n_L);
    ~SimpleGraph() = default;

    uint32_t getNoVertices() const override ;
    uint32_t getNoEdges() const override ;
    uint32_t getNoDistinctEdges() const override ;
    uint32_t getNoLabels() const override ;

    void addEdge(uint32_t from, uint32_t to, uint32_t edgeLabel) override ;
    void readFromContiguousFile(const std::string &fileName) override ;
};

#endif //QS_SIMPLEGRAPH_H