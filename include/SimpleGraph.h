//
// Created by Nikolay Yakovets on 2018-01-31.
//

#ifndef QS_SIMPLEGRAPH_H
#define QS_SIMPLEGRAPH_H

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <iostream>
#include <regex>
#include <fstream>
#include "Graph.h"

class SimpleGraph : public Graph {
public:

    typedef std::map<std::pair<uint32_t, uint32_t>, std::vector<uint32_t>> adjacency_matrix;
    adjacency_matrix adj;
    adjacency_matrix r_adj;

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

    adjacency_matrix::const_iterator begin(uint32_t n) const;
    adjacency_matrix::const_iterator rbegin(uint32_t n) const;
    adjacency_matrix::const_iterator end(uint32_t n) const;
    adjacency_matrix::const_iterator rend(uint32_t n) const;

};

#endif //QS_SIMPLEGRAPH_H
