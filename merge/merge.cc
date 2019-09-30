#include "merge.hh"

#include "edge_map.hh"
#include "exchange_pair.hh"
#include <length_calculator.hh>

#include <fstream>

namespace merge {

namespace {

template <typename AdjacentsContainer>
std::pair<primitives::point_id_t, primitives::point_id_t> sorted_pair(
    const AdjacentsContainer &adjacents,
    primitives::point_id_t i) {
    const auto &a = adjacents[i];
    return {std::min(a[0], a[1]), std::max(a[0], a[1])};
}

}  // namespace

std::pair<EdgeSet, EdgeSet> edge_differences(const Tour &tour1, const Tour &tour2) {
    const auto &adjacents1 = tour1.adjacents();
    const auto &adjacents2 = tour2.adjacents();
    EdgeSet diff1, diff2;
    for (primitives::point_id_t i{0}; i < adjacents1.size(); ++i) {
        const auto &sorted1 = sorted_pair(adjacents1, i);
        const auto &sorted2 = sorted_pair(adjacents2, i);
        if (sorted1 == sorted2) {
            continue;
        }
        const auto diff11 = sorted1.first != sorted2.first;
        const auto diff12 = sorted1.first != sorted2.second;
        const auto diff21 = sorted1.second != sorted2.first;
        const auto diff22 = sorted1.second != sorted2.second;
        if (diff11 and diff12) {
            const auto j = sorted1.first;
            diff1.emplace(std::min(i, j), std::max(i, j));
        }
        if (diff21 and diff22) {
            const auto j = sorted1.second;
            diff1.emplace(std::min(i, j), std::max(i, j));
        }
        if (diff11 and diff21) {
            const auto j = sorted2.first;
            diff2.emplace(std::min(i, j), std::max(i, j));
        }
        if (diff12 and diff22) {
            const auto j = sorted2.second;
            diff2.emplace(std::min(i, j), std::max(i, j));
        }
    }
    if (diff2.size() != diff1.size()) {
        throw std::logic_error("number of different edges are not the same between tours.");
    }
    return {std::move(diff1), std::move(diff2)};
}

ExchangePair disjoin(ExchangePair &base) {
    ExchangePair disjoined;
    auto new_points = disjoined.current.insert({base.current.pop_edge()});
    while (new_points) {
        new_points = disjoined.candidate.insert(*base.candidate.pop_edges(*new_points));
        if (not new_points) {
            break;
        }
        new_points = disjoined.current.insert(*base.current.pop_edges(*new_points));
    }
    return disjoined;
}

std::vector<ExchangePair> disjoin(const EdgeSet &current_edges, const EdgeSet &candidate_edges) {
    EdgeMap current(current_edges);
    EdgeMap candidate(candidate_edges);
    ExchangePair base{std::move(current), std::move(candidate)};
    std::vector<ExchangePair> exchange_pairs;
    while (not base.empty()) {
        exchange_pairs.push_back(std::move(disjoin(base)));
    }
    return exchange_pairs;
}

void merge(const Tour &current_tour, const Tour &candidate_tour) {
    const auto [old_edges, new_edges] = merge::edge_differences(current_tour, candidate_tour);
    std::cout << "edge diff count: " << old_edges.size() << std::endl;

    std::ofstream old_edge_file("output/old_edges.txt", std::ofstream::out);
    for (const auto &e : old_edges) {
        old_edge_file << e.first << ' ' << e.second << std::endl;
    }
    std::ofstream new_edge_file("output/new_edges.txt", std::ofstream::out);
    for (const auto &e : new_edges) {
        new_edge_file << e.first << ' ' << e.second << std::endl;
    }

    auto exchanges = disjoin(old_edges, new_edges);
    std::cout << exchanges.size() << " distinct exchange(s)." << std::endl;
    for (auto &ex : exchanges) {
        std::cout << "improvement: " << ex.compute_improvement(current_tour.x(), current_tour.y()) << std::endl;
    }
}

}  // namespace merge
