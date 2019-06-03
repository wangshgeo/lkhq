#pragma once

#include "BrokenEdge.h"
#include "KMove.h"
#include "Tour.h"
#include "primitives.h"

#include <algorithm> // sort
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace cycle_check {

// returns true if current swap does not break tour into multiple cycles.
// new edge i: (starts[i], ends[i])
// for p in removes: (p, next(p))
bool feasible(const Tour&
    , const std::vector<primitives::point_id_t>& starts
    , const std::vector<primitives::point_id_t>& ends
    , const std::vector<primitives::point_id_t>& removes);

inline bool feasible(const Tour& tour, const KMove& kmove)
{
    return feasible(tour, kmove.starts, kmove.ends, kmove.removes);
}

size_t count_cycles(const Tour&, const KMove&);
size_t count_cycles(const std::vector<BrokenEdge>& deleted_edges
    , const std::unordered_map<primitives::point_id_t, std::vector<primitives::point_id_t>>& new_edges);

void visit_cycle(const std::unordered_map<primitives::point_id_t, size_t>& sequence
    , const std::vector<BrokenEdge>& deleted_edges
    , std::unordered_set<primitives::point_id_t>& visited);

// sort deleted edges by sequence number.
std::vector<BrokenEdge> sorted_removes(const Tour&
    , const std::vector<primitives::point_id_t>& removes);

// map constituent points of deleted edges to sequence.
std::unordered_map<primitives::point_id_t, size_t>
compute_sequence(const std::vector<BrokenEdge>& deleted_edges);

// map points to up to 2 points making up new edges.
std::unordered_map<primitives::point_id_t, std::vector<primitives::point_id_t>>
compute_new_edge_connectivity(const std::vector<primitives::point_id_t>& starts
    , const std::vector<primitives::point_id_t>& ends);

} // namespace cycle_check

