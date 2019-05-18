#pragma once

#include "point_quadtree/Node.h"
#include "Neighborhood.h"

#include <algorithm> // min, max

namespace nearest_points {

using Pair = std::pair<Neighborhood, Neighborhood>;
using Neighborhoods = std::vector<Pair>;

inline Neighborhoods compute_starting(const point_quadtree::Node& root, const Tour& tour)
{
    Neighborhoods neighborhoods(tour.size());
    for (primitives::point_id_t i {0}; i < tour.size(); ++i)
    {
        const auto prev_length = tour.prev_length(i);
        const auto next_length = tour.length(i);
        const auto min_length = std::min(prev_length, next_length);
        neighborhoods[i].first = {root, tour, i, min_length};
        const auto max_length = std::max(prev_length, next_length);
        neighborhoods[i].second = {root, tour, i, max_length};
    }
    return neighborhoods;
}

} // namespace nearest_points


