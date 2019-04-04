#pragma once

#include <primitives.h>
#include <constants.h>

#include <vector>

namespace multicycle {
namespace simple_merge {

struct Swap
{
    std::vector<primitives::point_id_t> starts;
    std::vector<primitives::point_id_t> ends;
    std::vector<primitives::point_id_t> removed_edges;

    primitives::length_t improvement {0};
    primitives::length_t cost {0};

    bool valid() const
    {
        return starts.size() > 0;
    }
    bool operator<(const Swap& other) const
    {
        if (starts.size() == 0)
        {
            return starts.size() < other.starts.size();
        }
        if (improvement == other.improvement)
        {
            return other.cost < cost;
        }
        return improvement < other.improvement;
    }
};

} // namespace simple_merge
} // namespace multicycle
