#pragma once

#include "Pair.h"
#include "Swap.h"
#include <Tour.h>

#include <algorithm> // min

namespace perturbation {

inline bool is_valid_move(const Tour& tour
    , primitives::point_id_t i
    , primitives::point_id_t j
    , primitives::length_t current_length
    , primitives::length_t desired_cost
    , primitives::length_t& next_cost)
{
    auto new_length {tour.length_map().compute_length(i, j)};
    new_length += tour.length_map().compute_length(tour.next(i), tour.next(j));
    const auto target_length {current_length + desired_cost};
    if (new_length > target_length)
    {
        next_cost = std::min(next_cost, desired_cost + new_length - target_length);
    }
    return new_length == target_length;
}

inline std::vector<Swap> find_swaps(const Tour& tour
    , primitives::length_t cost
    , primitives::length_t& next_cost)
{
    std::vector<Swap> swaps;
    constexpr primitives::point_id_t start {0};
    // first segment cannot be compared with last segment.
    auto end {tour.prev(start)};
    const auto first_old_length {tour.length(start)};
    for (primitives::point_id_t i {tour.next(tour.next(start))}; i != end; i = tour.next(i))
    {
        const auto current_length {first_old_length + tour.length(i)};
        if (is_valid_move(tour, start, i, current_length, cost, next_cost))
        {
            swaps.push_back({start, i, cost});
        }
    }

    end = tour.prev(end);
    for (primitives::point_id_t i {tour.next(start)}; i != end; i = tour.next(i))
    {
        const auto first_old_length {tour.length(i)};
        auto j {tour.next(tour.next(i))};
        while (j != start)
        {
            const auto current_length {first_old_length + tour.length(j)};
            if (is_valid_move(tour, i, j, current_length, cost, next_cost))
            {
                swaps.push_back({i, j, cost});
            }
            j = tour.next(j);
        }
    }
    return swaps;
}

inline Tour perturbation_climb(const point_quadtree::Node& root
    , const std::vector<Swap>& swaps, const Tour& tour)
{
    const auto original_length {tour.length()};
    const auto original_points {tour.order()};
    for (const auto& swap : swaps)
    {
        auto new_tour = tour;
        new_tour.move(swap.a, swap.b);
        forward::optimize(root, new_tour, ignore_onsequential_moves);
        multicycle::optimize(root, new_tour);
        // solver::multi_climb(new_tour);
        if (new_tour.length() < original_length)
        {
            return new_tour;
        }
    }
    return tour;
}

inline Tour perturbation_climb(const point_quadtree::Node& root
    , const Tour& tour, primitives::length_t cost, primitives::length_t& next_cost)
{
    const auto swaps {find_swaps(tour, cost, next_cost)};
    return perturbation_climb(root, swaps, tour);
}

inline Tour perturbation_climb(const point_quadtree::Node& root, const Tour& tour)
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

} // namespace perturbation
