#pragma once

// Iteration scheme for perturbation and/or hill climbing at every point.

#include "hill_climb.h"
#include "primitives.h"

namespace sweep {

template <typename FinderType, typename PerturbationFunctor>
inline bool perturb(const Config& config
    , FinderType& finder
    , PerturbationFunctor& perturber)
{
    bool improved {false};
    auto best_length = finder.tour().length();
    for (primitives::point_id_t i {0}; i < finder.tour().size(); ++i)
    {
        std::cout << "trying " << i << " / " << finder.tour().size() << std::endl;
        auto original_tour = finder.tour();
        perturber(i, finder.tour());
        hill_climb::basic_hill_climb(config, finder);

        auto new_length = finder.tour().length();
        if (new_length < best_length)
        {
            std::cout << "improved to " << new_length << std::endl;
            best_length = new_length;
            improved = true;
        }
        else
        {
            finder.tour() = original_tour;
        }
    }
    return improved;
}

} // namespace sweep
