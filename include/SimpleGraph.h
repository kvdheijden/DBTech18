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

class SimpleVertex;

class SimpleEdge {
public:
    SimpleEdge(uint32_t label, const SimpleVertex& subject, const SimpleVertex& object);

    const uint32_t label;

    const SimpleVertex& source;
    const SimpleVertex& target;
};

typedef bool (*edge_sort_fcn)(const SimpleEdge* a, const SimpleEdge* b);

class SimpleVertex {
private:
    std::set<const SimpleEdge *, edge_sort_fcn> adj;
    std::set<const SimpleEdge *, edge_sort_fcn> r_adj;

public:
    const uint32_t label;

    const std::set<const SimpleEdge *, edge_sort_fcn>& outgoing() const;
    const std::set<const SimpleEdge *, edge_sort_fcn>& incoming() const;

    void insert_outgoing(const SimpleEdge& e);
    void insert_incoming(const SimpleEdge& e);

    explicit SimpleVertex(uint32_t l);
    bool operator<(const SimpleVertex& other) const;
    bool operator==(const SimpleVertex& other) const;
};

class SimpleGraph : public Graph {
private:
    uint32_t n_V;
    uint32_t n_L;
    uint32_t n_E;

    std::deque<SimpleVertex> V;
    std::deque<SimpleEdge> E;

public:

    SimpleGraph();

    uint32_t getNoVertices() const override;

    void setNoVertices(uint32_t v);

    uint32_t getNoEdges() const override;

    uint32_t getNoDistinctEdges() const override;

    uint32_t getNoLabels() const override;

    void setNoLabels(uint32_t l);

    void addEdge(uint32_t from, uint32_t to, uint32_t edgeLabel) override;

    void readFromContiguousFile(const std::string &fileName) override;

    SimpleVertex &getVertex(uint32_t i);
};

#endif //QS_SIMPLEGRAPH_H
