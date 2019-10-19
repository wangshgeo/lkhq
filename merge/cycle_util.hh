#pragma once

#include "edge.hh"
#include "edge_map.hh"
#include "exchange_pair.hh"
#include <tour.hh>
#include <kmove.hh>

#include <set>
#include <vector>

namespace merge {
namespace cycle_util {

// Takes an edge in (min, max) form and converts it to (i, next(i)) form.
Edge normalize_edge(const Tour &tour, const Edge& mm_edge);

// Takes edges in (min, max) form and converts them to (i, next(i)) form.
std::set<Edge> normalize_edges(const Tour& tour, const EdgeMap &edge_map);

void to_kmove(const Tour &best_tour, const Tour &candidate_tour, const ExchangePair &exchange_pair, KMove &kmove);

KMove to_kmove(const Tour &best_tour, const Tour &candidate_tour, const std::vector<ExchangePair> &exchange_pairs, const std::vector<size_t> &indices);

bool breaks_cycle(const Tour &best_tour, const Tour &candidate_tour, const std::vector<ExchangePair> &exchange_pairs, const std::vector<size_t> &indices);

bool breaks_cycle(const Tour &best_tour, const Tour &candidate_tour, const ExchangePair &exchange_pair);

size_t count_cycles(const Tour &best_tour, const Tour &candidate_tour, const std::vector<ExchangePair> &exchange_pairs, const std::vector<size_t> &indices);

std::vector<std::vector<primitives::point_id_t>> compute_cycles(const std::vector<primitives::point_id_t> &next);

}  // namespace cycle_util
}  // namespace merge

