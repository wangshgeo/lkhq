#pragma once

#include "multi_box.hh"
#include "primitives.hh"

#include <algorithm>
#include <iostream>
#include <vector>

namespace research {

inline void shorter_edge_opportunities(const PointSet &point_set, const Tour &tour) {
    std::vector<size_t> shorter_edge_count;
    for (primitives::point_id_t i{0}; i < point_set.size(); ++i) {
        const auto &next_length = tour.length(i);
        const auto &prev_length = tour.prev_length(i);
        const auto &max_length = std::max(prev_length, next_length);
        const auto &search_radius = max_length + 1;
        const auto &points = point_set.get_points(i, search_radius);
        std::vector<primitives::point_id_t> filtered_points;
        for (const auto &point : points) {
            if (point == tour.next(i) or point == tour.prev(i) or point == i) {
                continue;
            }
            if (point_set.length(point , i) >= max_length) {
                continue;
            }
            filtered_points.push_back(point);
        }
        if (filtered_points.empty()) {
            continue;
        }
        shorter_edge_count.push_back(filtered_points.size());
    }
    std::cout << "ratio of points with shorter edge opportunities: "
        << static_cast<double>(shorter_edge_count.size()) / point_set.size()
        << std::endl;
    const auto &sum = std::accumulate(std::cbegin(shorter_edge_count), std::cend(shorter_edge_count), static_cast<double>(0.0));
    std::cout << "average shorter edge opportunities (per point with any opportunities): "
        << sum / shorter_edge_count.size() << std::endl;
}

}  // namespace research

