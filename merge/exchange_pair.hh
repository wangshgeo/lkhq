#pragma once

#include "edge_map.hh"

#include <vector>
#include <optional>
#include <iostream>
#include <primitives.hh>

#include <length_calculator.hh>

namespace merge {

struct ExchangePair {
    EdgeMap current, candidate;
    std::optional<int> improvement{std::nullopt};

    bool empty() const { return current.empty() and candidate.empty(); }
    int compute_improvement(const std::vector<primitives::space_t> &x, const std::vector<primitives::space_t> &y);

    size_t edge_count() const {
        if (current.edge_count() != candidate.edge_count()) {
            std::cout << current.edge_count() << ", " << candidate.edge_count() << std::endl;
            throw std::logic_error("inconsistent edge count in exchange pair.");
        }
        return current.edge_count();
    }

 private:
    template <typename EdgePair>
    int cost(const EdgePair &edge_pair, const LengthCalculator &calc) const;
};

template <typename EdgePair>
int ExchangePair::cost(const EdgePair &edge_pair, const LengthCalculator &calc) const {
    int cost{0};
    const auto &first_edge = edge_pair.first;
    if (first_edge) {
        cost += calc(first_edge->first, first_edge->second);
    }
    const auto &second_edge = edge_pair.second;
    if (second_edge) {
        cost += calc(second_edge->first, second_edge->second);
    }
    return cost;
}

}  // namespace merge

