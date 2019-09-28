#pragma once

#include "GenericFinder.h"
#include "primitives.hh"

#include <algorithm> // max, random_shuffle
#include <iostream>
#include <vector>

namespace hill_climb {

class RandomFinder : public GenericFinder<RandomFinder>
{
    using base = GenericFinder<RandomFinder>;
public:
    using base::base;
    std::vector<primitives::point_id_t> search_neighborhood(primitives::point_id_t p);
    bool final_new_edge() const;
    void final_stats() const { std::cout << "max k: " << m_max_kdepth << std::endl; }

    auto max_kdepth() const { return m_max_kdepth; }

private:
    size_t m_max_kdepth {0};

};

}  // namespace hill_climb
