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
    SimpleEdge(uint32_t label, const std::shared_ptr<SimpleVertex>& subject, const std::shared_ptr<SimpleVertex>& object);

    const uint32_t label;

    const std::shared_ptr<SimpleVertex>& source;
    const std::shared_ptr<SimpleVertex>& target;
};

typedef bool (*edge_sort_fcn)(const std::shared_ptr<SimpleEdge>& a, const std::shared_ptr<SimpleEdge>& b);

class SimpleVertex {
private:
    std::set<std::shared_ptr<SimpleEdge>, edge_sort_fcn> adj;
    std::set<std::shared_ptr<SimpleEdge>, edge_sort_fcn> r_adj;

public:
    const uint32_t label;

    const std::set<std::shared_ptr<SimpleEdge>, edge_sort_fcn>& outgoing() const;
    const std::set<std::shared_ptr<SimpleEdge>, edge_sort_fcn>& incoming() const;

    void insert_outgoing(const std::shared_ptr<SimpleEdge>& e);
    void insert_incoming(const std::shared_ptr<SimpleEdge>& e);

    explicit SimpleVertex(uint32_t l);
    bool operator<(const SimpleVertex& other) const;
    bool operator==(const SimpleVertex& other) const;
};

class SimpleGraph : public Graph {
private:
    uint32_t n_V;
    uint32_t n_L;
    uint32_t n_E;

    std::vector<std::shared_ptr<SimpleVertex>> V;
    std::vector<std::shared_ptr<SimpleEdge>> E;

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
