#pragma once

#include "GenericFinder.h"
#include "primitives.h"

#include <vector>

class OptimalFinder : public GenericFinder<OptimalFinder>
{
    using base = GenericFinder<OptimalFinder>;
public:
    using base::base;
    std::vector<primitives::point_id_t> search_neighborhood(primitives::point_id_t p) const;
    bool final_new_edge() const;
    void final_stats() const {}

};


