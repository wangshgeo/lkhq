#pragma once

// Iteration scheme for perturbation and/or hill climbing at every point.

#include "primitives.h"

namespace sweep {

template <typename FinderType, typename PerturbationFunctor>
inline bool perturb(FinderType& finder, const PerturbationFunctor& perturber)
{
    bool improved {false};
    for (primitives::point_id_t i {0}; i < finder.tour().size(); ++i)
    {
        const auto starts = perturber(i, finder.tour());
        const auto kmove = finder.find_best(starts);
        if (kmove)
        {
            finder.tour().swap(kmove->starts, kmove->ends, kmove->removes);
            improved = true;
        }
    }
    return improved;
}

} // namespace sweep
