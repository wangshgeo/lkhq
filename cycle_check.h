#pragma once

#include "BrokenEdge.h"
#include "Tour.h"
#include "primitives.h"

#include <algorithm> // sort
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace cycle_check {

// returns true if current swap does not break tour into multiple cycles.
// new edge i: (starts[i], ends[i])
// for p in removes: (p, next(p))
bool feasible(const Tour&
    , const std::vector<primitives::point_id_t>& starts
    , const std::vector<primitives::point_id_t>& ends
    , const std::vector<primitives::point_id_t>& removes);

} // namespace cycle_check

