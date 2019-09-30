#pragma once

#include <utility>
#include <vector>

#include "edge_set.hh"
#include "edge_map.hh"

namespace merge {

using ExchangeSet = std::pair<EdgeSet, EdgeSet>;

class ExchangeSet {
 public:
    ExchangeSet(const EdgeSet &edges, const EdgeSet &candidate_edges)
        : edges_(edges)
        , candidate_edges_(candidate_edges)
        , edge_map_(edges)
        , candidate_edge_map_(candidate_edges) {}

    Edge pop_edge const {
        auto it = std::cbegin(edges_);
        auto edge = *it;
        edges_.erase(it);
        return edge;
    }

    std::vector<Edge> pop_edges(const std::vector<primitives::point_id_t> &points) {
    }

 private:
     EdgeSet edges_, candidate_edges_;
     EdgeMap edge_map_, candidate_edge_map_;
};

}  // namespace merge
