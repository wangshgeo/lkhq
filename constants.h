#pragma once

#include "primitives.h"

namespace constants {

constexpr auto invalid_point {std::numeric_limits<primitives::point_id_t>::max()};
constexpr auto invalid_cycle {-1};
constexpr auto max_length {std::numeric_limits<primitives::length_t>::max()};

constexpr int save_period {1000};

// TODO: optimize this, and/or allow for point placement at leaf nodes higher than max_tree_depth.
constexpr primitives::depth_t max_tree_depth{15}; // maximum quadtree depth / level.

constexpr bool verbose {false};
constexpr bool write_best {true};
constexpr bool print_local_optima {true};
constexpr bool print_iterations {true};
constexpr bool print_improvements {true};
constexpr bool verify {true};

constexpr bool quit_after_improvement {false};

} // namespace constants
