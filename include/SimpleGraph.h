//
// Created by Nikolay Yakovets on 2018-01-31.
//

#ifndef QS_SIMPLEGRAPH_H
#define QS_SIMPLEGRAPH_H

#include <set>
#include <vector>
#include <fstream>
#include <regex>

#include "Graph.h"

class SimpleGraph : public Graph {
private:
    uint32_t L;
    uint32_t V;
    uint32_t E;

    typedef std::map<std::pair<uint32_t, uint32_t>, std::vector<uint32_t>> adjacency_matrix;
    adjacency_matrix adj;
    adjacency_matrix r_adj;

    void setNoVertices(uint32_t v);

    void setNoEdges(uint32_t e);

    void setNoLabels(uint32_t l);

public:

    SimpleGraph();

    ~SimpleGraph() = default;

    explicit SimpleGraph(uint32_t n_L);

    SimpleGraph(uint32_t n_V, uint32_t n_L);

    uint32_t getNoVertices() const override;

    uint32_t getNoEdges() const override;

    uint32_t getNoDistinctEdges() const override;

    uint32_t getNoLabels() const override;

    void addEdge(uint32_t from, uint32_t to, uint32_t edgeLabel) override;

    void readFromContiguousFile(const std::string &fileName) override;

    adjacency_matrix::const_iterator begin(uint32_t n) const;
    adjacency_matrix::const_iterator end(uint32_t n) const;
    adjacency_matrix::const_iterator rbegin(uint32_t n) const;
    adjacency_matrix::const_iterator rend(uint32_t n) const;
};

#endif //QS_SIMPLEGRAPH_H
