//
// Created by Nikolay Yakovets on 2018-01-31.
//

#ifndef QS_SIMPLEGRAPH_H
#define QS_SIMPLEGRAPH_H

#include <algorithm>
#include <fstream>
#include <regex>
#include <vector>

#include "Graph.h"

class SimpleGraph : public Graph {
private:
    std::vector<std::vector<std::pair<uint32_t,uint32_t>>> adj;
    std::vector<std::vector<std::pair<uint32_t,uint32_t>>> reverse_adj; // vertex adjacency list
protected:
    uint32_t V;
    uint32_t L;

public:

    SimpleGraph() : V(0), L(0) {};
    ~SimpleGraph() = default;
    explicit SimpleGraph(uint32_t n);

    uint32_t getNoVertices() const override ;
    uint32_t getNoEdges() const override ;
    uint32_t getNoDistinctEdges() const override ;
    uint32_t getNoLabels() const override ;

    void addEdge(uint32_t from, uint32_t to, uint32_t edgeLabel) override ;
    void readFromContiguousFile(const std::string &fileName) override ;

    void setNoVertices(uint32_t n);
    void setNoLabels(uint32_t noLabels);

    int outDegree(uint32_t n) const;
    int inDegree(uint32_t n) const;

    std::vector<std::pair<uint32_t, uint32_t>>& forward(uint32_t n);
    std::vector<std::pair<uint32_t, uint32_t>>& reverse(uint32_t n);

    static uint32_t getLabel(const std::pair<uint32_t, uint32_t>& labelTarget);
    static uint32_t getTarget(const std::pair<uint32_t, uint32_t>& labelTarget);
};

#endif //QS_SIMPLEGRAPH_H