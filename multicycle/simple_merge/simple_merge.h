#pragma once

#include "Swap.h"
#include <point_quadtree/Node.h>
#include <Tour.h>
#include <constants.h>
#include <primitives.h>

#include <vector>

namespace multicycle {
namespace simple_merge {

inline void search_neighbors(Tour& tour
    , const primitives::point_id_t new_start
    , const primitives::point_id_t new_removal
    , const primitives::point_id_t global_swap_end
    , const primitives::length_t removed_length
    , const primitives::length_t added_length
    , Swap& trial_swap
    , Swap& best_swap)
{
    const auto deletion {tour.length(new_removal)};
    const auto closing_length {tour.length(new_start, global_swap_end)};
    const auto total_added {closing_length + added_length};
    const auto total_removed {removed_length + deletion};
    const bool improving {total_removed > total_added};
    // don't need to check if new edge is created because new_start is in different cycle.
    trial_swap.starts.push_back(new_start);
    trial_swap.ends.push_back(global_swap_end);
    trial_swap.removed_edges.push_back(new_removal);
    if (improving)
    {
        trial_swap.improvement = total_removed - total_added;
    }
    else
    {
        trial_swap.cost = total_added - total_removed;
    }
    best_swap = std::max(best_swap, trial_swap);
    trial_swap.starts.pop_back();
    trial_swap.ends.pop_back();
    trial_swap.removed_edges.pop_back();
    trial_swap.improvement = 0;
    trial_swap.cost = 0;
}

inline void search_both_sides(Tour& tour
    , const primitives::point_id_t global_swap_end
    , const primitives::length_t removed_length
    , const primitives::length_t added_length
    , Swap& trial_swap
    , Swap& best_swap)
{
    // each point p in removed_edges represents the edge (p, next(p)).
    // here, starts.size() == ends.size(), and removed_edges.size() == ends.size()
    const auto prev {tour.prev(trial_swap.ends.back())};
    auto new_start {prev};
    auto new_removal {prev};
    const auto has_prev_edge
    {
        std::find(std::cbegin(trial_swap.removed_edges), std::cend(trial_swap.removed_edges), new_removal)
            == std::cend(trial_swap.removed_edges)
    };
    if (has_prev_edge)
    {
        search_neighbors(tour
            , new_start, new_removal
            , global_swap_end
            , removed_length, added_length
            , trial_swap, best_swap);
    }
    const auto next {tour.next(trial_swap.ends.back())};
    new_start = next;
    new_removal = trial_swap.ends.back();
    const auto has_next_edge
    {
        std::find(std::cbegin(trial_swap.removed_edges), std::cend(trial_swap.removed_edges), new_removal)
            == std::cend(trial_swap.removed_edges)
    };
    if (has_next_edge)
    {
        search_neighbors(tour
            , new_start, new_removal
            , global_swap_end
            , removed_length, added_length
            , trial_swap, best_swap);
    }
}

inline void start_search(Tour& tour
    , const point_quadtree::Node& root
    , const primitives::length_t search_radius_factor
    , const primitives::cycle_id_t excluded_cycle_id
    , const primitives::point_id_t swap_start
    , const primitives::point_id_t removed_edge
    , const primitives::point_id_t swap_end
    , Swap& best_swap)
{
    const auto removed_length {tour.length(removed_edge)};
    Swap trial_swap;
    trial_swap.starts.push_back(swap_start);
    trial_swap.removed_edges.push_back(removed_edge);
    const auto search_box
    {
        tour.search_box(swap_start, search_radius_factor * (removed_length + tour.max_outgroup_length() + 2))
    };
    std::vector<primitives::point_id_t> points;
    root.get_points(swap_start, search_box, points);
    for (auto p : points)
    {
        if (tour.cycle_id(p) == excluded_cycle_id)
        {
            continue;
        }
        const auto added_length {tour.length(swap_start, p)};
        trial_swap.ends.push_back(p);
        search_both_sides(tour
            , swap_end
            , removed_length
            , added_length
            , trial_swap
            , best_swap);
        trial_swap.ends.pop_back();
    }
}

// finds a 2-opt merge to reduce the number of cycles by 1.
inline Swap find_merge_swap(const point_quadtree::Node& root
    , Tour& tour
    , primitives::length_t search_radius_factor = 1)
{
    constexpr primitives::point_id_t start {0};
    const primitives::cycle_id_t base_cycle_id {tour.cycle_id(start)};
    primitives::point_id_t i {start};
    Swap best_swap;
    do
    {
        // delete prev segment.
        start_search(tour, root
            , search_radius_factor
            , base_cycle_id
            , i
            , tour.prev(i)
            , tour.prev(i)
            , best_swap);
        // delete next segment.
        start_search(tour, root
            , search_radius_factor
            , base_cycle_id
            , i
            , i
            , tour.next(i)
            , best_swap);
        i = tour.next(i);
    } while (i != start);
    return best_swap;
}

inline void merge_once(const point_quadtree::Node& root, Tour& tour)
{
    // mark cycle that has "start".
    constexpr primitives::point_id_t start {0};
    std::vector<bool> first_group(tour.size(), false);
    auto current {start};
    size_t visited {0};
    do
    {
        first_group[current] = true;
        ++visited;
        if (visited > tour.size())
        {
            std::cout << __func__ << ": error: could not traverse back to start." << std::endl;
            std::abort();
        }
        current = tour.next(current);
    } while (current != start);
    if (visited == tour.size()) // quit if single cycle.
    {
        return;
    }
    // find merging swap (if not found, increase search radius until found).
    auto swap {find_merge_swap(root, tour)};
    primitives::length_t search_radius_factor {1};
    while (not swap.valid())
    {
        // std::cout << "search radius factor: " << search_radius_factor << std::endl;
        search_radius_factor <<= 1;
        if (search_radius_factor > 100000)
        {
            std::cout << __func__ << ": error: search radius unreasonably large." << std::endl;
            std::abort();
        }
        swap = find_merge_swap(root, tour, search_radius_factor);
    }
    tour.multicycle_swap(swap.starts, swap.ends, swap.removed_edges);
}

} // namespace simple_merge
} // namespace multicycle
