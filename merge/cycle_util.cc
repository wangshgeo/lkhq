#include "cycle_util.hh"

#include <cycle_check.hh>

#include <algorithm>

namespace merge {
namespace cycle_util {

// Takes an edge in (min, max) form and converts it to (i, next(i)) form.
Edge normalize_edge(const Tour &tour, const Edge& mm_edge) {
    if (tour.next(mm_edge.first) == mm_edge.second) {
        return mm_edge;
    }
    if (tour.next(mm_edge.second) == mm_edge.first) {
        return {mm_edge.second, mm_edge.first};
    }
    throw std::logic_error("invalid edge.");
}

// Takes edges in (min, max) form and converts them to (i, next(i)) form.
std::set<Edge> normalize_edges(const Tour& tour, const EdgeMap &edge_map) {
    std::set<Edge> normalized;
    for (const auto &pair : edge_map.map()) {
        if (pair.second.first) {
            normalized.insert(normalize_edge(tour, *pair.second.first));
        }
        if (pair.second.second) {
            normalized.insert(normalize_edge(tour, *pair.second.second));
        }
    }
    return normalized;
}

void to_kmove(const Tour &best_tour, const Tour &candidate_tour, const ExchangePair &exchange_pair, KMove &kmove) {
    const auto &remove_set = normalize_edges(best_tour, exchange_pair.current);
    const auto &add_set = normalize_edges(candidate_tour, exchange_pair.candidate);
    for (const auto &edge : remove_set) {
        kmove.removes.push_back(edge.first);
    }
    for (const auto &edge : add_set) {
        kmove.starts.push_back(edge.first);
        kmove.ends.push_back(edge.second);
    }
}

KMove to_kmove(const Tour &best_tour, const Tour &candidate_tour, const std::vector<ExchangePair> &exchange_pairs, const std::vector<size_t> &indices) {
    KMove kmove;
    for (const auto &i : indices) {
        const auto &e = exchange_pairs[i];
        to_kmove(best_tour, candidate_tour, e, kmove);
    }
    return kmove;
}

bool breaks_cycle(const Tour &best_tour, const Tour &candidate_tour, const std::vector<ExchangePair> &exchange_pairs, const std::vector<size_t> &indices) {
    const auto kmove = to_kmove(best_tour, candidate_tour, exchange_pairs, indices);
    return cycle_check::breaks_cycle(best_tour, kmove);
}

bool breaks_cycle(const Tour &best_tour, const Tour &candidate_tour, const ExchangePair &exchange_pair) {
    KMove kmove;
    to_kmove(best_tour, candidate_tour, exchange_pair, kmove);
    return cycle_check::breaks_cycle(best_tour, kmove);
}

size_t count_cycles(const Tour &best_tour, const Tour &candidate_tour, const std::vector<ExchangePair> &exchange_pairs, const std::vector<size_t> &indices) {
    const auto kmove = to_kmove(best_tour, candidate_tour, exchange_pairs, indices);
    return cycle_check::count_cycles(best_tour, kmove);
}

std::vector<std::vector<primitives::point_id_t>> compute_cycles(const std::vector<primitives::point_id_t> &next) {
    std::unordered_set<primitives::point_id_t> remaining;
    for (primitives::point_id_t i{0}; i < next.size(); ++i) {
        remaining.insert(i);
    }
    primitives::point_id_t current{0};
    std::vector<std::vector<primitives::point_id_t>> cycles;
    cycles.emplace_back();
    do {
        cycles.back().push_back(current);
        remaining.erase(current);
        current = next[current];
        if (current == cycles.back().front()) {
            if (remaining.empty()) {
                break;
            }
            cycles.emplace_back();
            current = *std::begin(remaining);
        }
    } while (not remaining.empty());
    std::unordered_set<primitives::point_id_t> seen; 
    for (const auto &cycle : cycles) {
        for (const auto &i : cycle) {
            if (seen.find(i) != std::cend(seen)) {
                throw std::logic_error("invalid cycles; repeated point.");
            }
            seen.insert(i);
        }
    }
    if (seen.size() != next.size()) {
        throw std::logic_error("missing points in cycles.");
    }
    return cycles;
}

}  // namespace cycle_util
}  // namespace merge
