#pragma once

#include <constants.h>
#include <primitives.h>

namespace multicycle {
namespace break_detection {

struct BrokenEdge
{
    // Direction of traversal is first to second.
    primitives::point_id_t first {constants::invalid_point};
    primitives::point_id_t second {constants::invalid_point};
    primitives::point_id_t sequence {0}; // relative order of this edge to other edges.
};

} // namespace break_detection
} // namespace multicycle

