#pragma once

#include "FeasibleFinder.h"

#include <algorithm> // fill
#include <unordered_set>
#include <vector>

class MaskedFeasibleFinder : public FeasibleFinder
{
public:

    bool find_best();

private:
    std::unordered_set<primitives::point_id_t> m_active;
    std::vector<std::vector<primitives::point_id_t>> m_neighborhoods;

    void activate_nearest(primitives::point_id_t i) //, size_t min_points)
    {
        m_active.clear();
        m_active.insert(i);

    }

};



