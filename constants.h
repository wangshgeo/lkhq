#pragma once

#include "primitives.h"

#include <limits>

namespace constants {

constexpr auto invalid_point {std::numeric_limits<primitives::point_id_t>::max()};
constexpr auto invalid_cycle {-1};

constexpr primitives::depth_t max_tree_depth{15}; // maximum quadtree depth / level.

} // namespace constants
