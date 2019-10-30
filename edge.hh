#pragma once

// Simple direction-agnostic representation of an edge.

#include "primitives.hh"

#include <algorithm>

namespace edge {

using Edge = std::pair<primitives::point_id_t, primitives::point_id_t>;

inline Edge make_edge(primitives::point_id_t a, primitives::point_id_t b) {
    return {std::min(a, b), std::max(a, b)};
}

}  // namespace edge
