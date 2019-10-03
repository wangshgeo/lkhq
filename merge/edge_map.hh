#pragma once

// Maps points to their edges.

#include "edge.hh"
#include "edge_set.hh"

#include <primitives.hh>

#include <unordered_map>
#include <vector>
#include <optional>

namespace merge {

struct EdgeMap {
    using Point = primitives::point_id_t;
    using Points = std::vector<Point>;
    using Edges = std::vector<Edge>;
    using IncidentEdges = std::pair<std::optional<Edge>, std::optional<Edge>>;
 public:
    EdgeMap() = default;
    EdgeMap(const EdgeSet &edge_set);

    std::optional<Points> insert(const Edge &edge);
    std::optional<Points> insert(const Edges &edges);
    Edge pop_edge();
    std::optional<Edges> pop_edges(const Points &points);

    bool empty() const { return map_.empty(); }
    const auto &map() const { return map_; }

    size_t edge_count() const { return edge_count_; }

 private:
    std::optional<Edges> pop_edges(const Point &i);

    // Returns true if i is new.
    bool insert(primitives::point_id_t i, const Edge &edge);

    // remove edge entries for both points in the edge.
    void remove_edges(const Edge &edge);
    void remove_edge(primitives::point_id_t i, const Edge &edge);

    // Maps each point to the edges that said point is a part of. Each edge is stored twice, once for each point.
    std::unordered_map<Point, IncidentEdges> map_;

    size_t edge_count_{0};

};

}  // namespace merge
