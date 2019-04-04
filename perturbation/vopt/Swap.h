#pragma once

#include <constants.h>
#include <primitives.h>

namespace vopt {

struct Swap
{
    primitives::point_id_t v {constants::invalid_point}; // vertex from which both edges are removed.
    primitives::point_id_t n {constants::invalid_point}; // point from which only one edge is removed, (n, next(n)).
    primitives::length_t improvement {0};
};

} // namespace vopt
