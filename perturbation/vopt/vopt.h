#pragma once

#include "Swap.h"
#include <TourModifier.h>
#include <constants.h>
#include <primitives.h>

#include <vector>

namespace vopt {

inline primitives::length_t compute_improvement(const TourModifier& tour
    , primitives::point_id_t v
    , primitives::point_id_t n
    , primitives::length_t known_current_length
    , primitives::length_t known_new_length)
{
    known_new_length += tour.length_map().compute_length(v, n);
    if (known_new_length >= known_current_length)
    {
        return 0;
    }
    known_new_length += tour.length_map().compute_length(v, tour.next(n));
    if (known_new_length >= known_current_length)
    {
        return 0;
    }
    known_current_length += tour.length(n);
    if (known_new_length >= known_current_length)
    {
        return 0;
    }
    return known_current_length - known_new_length;
}

inline Swap first_improvement(const TourModifier& tour)
{
    constexpr primitives::point_id_t v_start {0};
    // the only restrictions on comparison with point p is prev(p) and p itself.
    primitives::point_id_t v {v_start};
    do
    {
        const auto start {tour.next(v)};
        const auto end {tour.prev(v)};
        const auto known_new_length {tour.length_map().compute_length(tour.prev(v), tour.next(v))};
        const auto known_current_length {tour.length(v) + tour.prev_length(v)};
        for (primitives::point_id_t n {start}; n != end; n = tour.next(n))
        {
            const auto improvement {compute_improvement(tour, v, n, known_current_length, known_new_length)};
            if (improvement > 0)
            {
                return {v, n, improvement};
            }
        }
        v = tour.next(v);
    } while (v != v_start);
    return {};
}

inline bool hill_climb(TourModifier& tour)
{
    bool improved {false};
    auto move {first_improvement(tour)};
    if (move.improvement > 0)
    {
        improved = true;
    }
    int iteration{1};
    while (move.improvement > 0)
    {
        tour.vmove(move.v, move.n);
        if (constants::verbose)
        {
            auto length {tour.length()};
            std::cout << "Iteration " << iteration
                << " tour length: " << length
                << " (step improvement: " << move.improvement << ")\n";
        }
        move = first_improvement(tour);
        if (move.improvement > 0)
        {
            improved = true;
        }
        ++iteration;
    }
    return improved;
}

} // namespace vopt


