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

class SimpleEdge {
public:
    SimpleEdge(uint32_t label, uint32_t subject, uint32_t object);

    const uint32_t label;

    const uint32_t source;
    const uint32_t target;
};

class SimpleGraph : public Graph {
private:
    uint32_t L;
    uint32_t V;
    uint32_t E;

    std::unordered_map<uint32_t, std::vector<std::shared_ptr<SimpleEdge>>> adj;
    std::unordered_map<uint32_t, std::vector<std::shared_ptr<SimpleEdge>>> r_adj;

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

    std::vector<std::shared_ptr<SimpleEdge>>& getAdjacency(uint32_t n);
    std::vector<std::shared_ptr<SimpleEdge>>& getReverseAdjacency(uint32_t n);
};

#endif //QS_SIMPLEGRAPH_H
