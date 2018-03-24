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

class SimpleVertex {
private:
    std::vector<const SimpleEdge *> adj;
    std::vector<const SimpleEdge *> r_adj;

public:
    const uint32_t label;

    const std::vector<const SimpleEdge *>& outgoing() const;
    const std::vector<const SimpleEdge *>& incoming() const;

    void insert_outgoing(const SimpleEdge& e);
    void insert_incoming(const SimpleEdge& e);

    uint32_t inDegree() const;
    uint32_t outDegree() const;

    explicit SimpleVertex(uint32_t l);
    bool operator<(const SimpleVertex& other) const;
    bool operator==(const SimpleVertex& other) const;
    bool operator!=(const SimpleVertex& other) const;
};

class SimpleGraph : public Graph {
private:
    uint32_t L;

    std::deque<SimpleVertex> V;
    std::deque<SimpleEdge> E;

public:

    SimpleGraph();

    explicit SimpleGraph(uint32_t n_L);

    SimpleGraph(uint32_t n_V, uint32_t n_L);

    explicit SimpleGraph(const std::string& fileName);

    uint32_t getNoVertices() const override;

    void setNoVertices(uint32_t v);

    uint32_t getNoEdges() const override;

    uint32_t getNoDistinctEdges() const override;

    uint32_t getNoLabels() const override;

    void addEdge(uint32_t from, uint32_t to, uint32_t edgeLabel) override;

    void readFromContiguousFile(const std::string &fileName) override;

    SimpleVertex &getVertex(uint32_t i);
};

#endif //QS_SIMPLEGRAPH_H
