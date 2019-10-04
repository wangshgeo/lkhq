#pragma once

#include "edge_set.hh"
#include "exchange_pair.hh"
#include "tour.hh"
#include "primitives.hh"

#include <algorithm>
#include <vector>
#include <set>
#include <utility>

namespace merge {

using ExchangeSet = std::pair<EdgeSet, EdgeSet>;

std::pair<EdgeSet, EdgeSet> edge_differences(const Tour &tour1, const Tour &tour2);

// Removes a disjoint ExchangePair from base.
ExchangePair disjoin(ExchangePair &base);
std::vector<ExchangePair> disjoin(const EdgeSet &current, const EdgeSet &candidate);

void merge(Tour &current_tour, const Tour &candidate_tour);

}  // namespace merge
