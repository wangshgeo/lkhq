#pragma once

#include <primitives.h>

#include <vector>

namespace perturbation {

struct Swap
{
    std::vector<primitives::point_id_t> starts;
    std::vector<primitives::point_id_t> ends;
    std::vector<primitives::point_id_t> removed_edges;
};

} // namespace perturbation
