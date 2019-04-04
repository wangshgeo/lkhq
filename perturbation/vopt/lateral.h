#pragma once

#include <Pair.h>
#include "Swap.h"
#include "TourModifier.h"
#include "solver.h"

#include <algorithm> // min

namespace vopt {
namespace lateral {

inline bool is_valid_move(const TourModifier& tour
    , primitives::point_id_t v
    , primitives::point_id_t n
    , primitives::length_t known_current_length
    , primitives::length_t known_new_length
    , primitives::length_t desired_cost
    , primitives::length_t& next_cost)
{
    known_new_length += tour.length_map().compute_length(v, n);
    known_new_length += tour.length_map().compute_length(v, tour.next(n));
    known_current_length += tour.length(n);
    const auto target_length {known_current_length + desired_cost};
    if (known_new_length > target_length)
    {
        next_cost = std::min(next_cost, desired_cost + known_new_length - target_length);
    }
    return known_new_length == target_length;
}

inline std::vector<Swap> find_swaps(const TourModifier& tour
    , primitives::length_t cost
    , primitives::length_t& next_cost)
{
    std::vector<Swap> swaps;
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
            if (is_valid_move(tour, v, n, known_current_length, known_new_length, cost, next_cost))
            {
                swaps.push_back({v, n, cost});
            }
        }
        v = tour.next(v);
    } while (v != v_start);
    return swaps;
}

inline Swap restricted_first_improvement(const TourModifier& tour, const primitives::point_id_t v_restriction, const Segment& join_restriction)
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
                const Segment deletion(n, tour.next(n));
                if (v != v_restriction or join_restriction != deletion)
                {
                    return {v, n, improvement};
                }
            }
        }
        v = tour.next(v);
    } while (v != v_start);
    return {};
}

inline TourModifier perturbation_climb(const std::vector<Swap>& swaps, const TourModifier& tour)
{
    const auto original_length {tour.length()};
    for (const auto& swap : swaps)
    {
        auto new_tour = tour;
        new_tour.vmove(swap.v, swap.n);
        const Segment join_restriction(tour.prev(swap.v), tour.next(swap.v));
        while (true)
        {
            const auto new_swap {restricted_first_improvement(new_tour, swap.v, join_restriction)};
            if (new_swap.improvement == 0)
            {
                break;
            }
            new_tour.vmove(new_swap.v, new_swap.n);
        }
        solver::multi_climb(new_tour);
        if (new_tour.length() < original_length)
        {
            return new_tour;
        }
    }
    return tour;
}

inline TourModifier perturbation_climb(const TourModifier& tour, primitives::length_t cost, primitives::length_t& next_cost)
{
    const auto swaps {find_swaps(tour, cost, next_cost)};
    return perturbation_climb(swaps, tour);
}

inline TourModifier perturbation_climb(const TourModifier& tour)
{
    const auto original_length {tour.length()};
    primitives::length_t current_cost {0};
    while (true)
    {
        std::cout << "trying perturbation cost: " << current_cost << std::endl;
        primitives::length_t next_cost {constants::invalid_length};
        const auto new_tour {perturbation_climb(tour, current_cost, next_cost)};
        if (new_tour.length() < original_length)
        {
            return new_tour;
        }
        if (next_cost == constants::invalid_length)
        {
            break;
        }
        current_cost = next_cost;
    }
    std::cout << "No more perturbations left to try." << std::endl;
    return tour;
}

} // namespace lateral
} // namespace vopt
