#include "multicycle.h"

namespace multicycle {

bool search_neighbors(Tour& tour
    , const point_quadtree::Node& root
    , const primitives::point_id_t new_start
    , const primitives::point_id_t new_removal
    , const primitives::point_id_t global_swap_end
    , const primitives::length_t removed_length
    , const primitives::length_t added_length
    , std::vector<primitives::point_id_t>& starts
    , std::vector<primitives::point_id_t>& ends
    , std::vector<primitives::point_id_t>& removed_edges)
{
    /*
    std::cout << "removed size: " << removed_edges.size() << std::endl;
    std::set<primitives::point_id_t> seen;
    for (auto p : starts)
    {
        if (seen.find(p) != std::cend(seen))
        {
            std::cout << "repeat";
        }
        std::cout << p << " ";
        seen.insert(p);
    }
    std::cout << new_start;
    std::cout << std::endl;
    */

    const auto deletion {tour.length(new_removal)};

    // first check if can close.
    const auto closing_length {tour.length(new_start, global_swap_end)};
    const bool improving {removed_length + deletion > closing_length + added_length};
    if (improving)
    {
        if (new_start != tour.prev(global_swap_end) and new_start != tour.next(global_swap_end))
        {
            starts.push_back(new_start);
            ends.push_back(global_swap_end);
            removed_edges.push_back(new_removal);
            if (multicycle::break_detection::feasible(tour, starts, ends, removed_edges))
            {
                // std::cout << "found feasible" << std::endl;
                return true;
            }
            else
            {
                // std::cout << "nonsequential" << std::endl;
                if (test_nonfeasible(root, tour, starts, ends, removed_edges))
                {
                    return true;
                }
            }
            starts.pop_back();
            ends.pop_back();
            removed_edges.pop_back();
        }
    }

    std::vector<primitives::point_id_t> points;
    root.get_points(new_start, tour.search_box(new_start, tour.length(new_removal) + 1), points);
    const auto minimum_sequence {tour.sequence(new_start, removed_edges.front())};
    for (auto p : points)
    {
        // TODO: filter out higher-sequence points.
        const auto p_sequence {tour.sequence(p, removed_edges.front())};
        if (p_sequence <= minimum_sequence)
        {
            continue;
        }

        // check easy exclusion cases.
        const bool closing {p == global_swap_end};
        const bool neighboring {p == tour.next(new_start) or p == tour.prev(new_start)};
        const bool self {p == new_start};
        if (self or closing or neighboring)
        {
            continue;
        }

        // check if worth considering.
        const auto addition {tour.length(new_start, p)};
        const auto no_gain {addition >= deletion};
        if (no_gain)
        {
            continue;
        }

        // check if repeating move.
        const bool has_start {std::find(std::cbegin(starts), std::cend(starts), new_start) != std::cend(starts)};
        const bool has_end {std::find(std::cbegin(ends), std::cend(ends), p) != std::cend(ends)};
        if (has_start and has_end)
        {
            continue;
        }

        // check backtrack.
        const bool backtrack {starts.back() == p};
        if (backtrack)
        {
            continue;
        }

        starts.push_back(new_start);
        ends.push_back(p);
        removed_edges.push_back(new_removal);
        if (search_both_sides(tour, root
            , global_swap_end
            , removed_length + deletion
            , added_length + addition
            , starts
            , ends
            , removed_edges))
        {
            return true;
        }
        starts.pop_back();
        ends.pop_back();
        removed_edges.pop_back();
    }
    return false;
}

bool search_both_sides(Tour& tour
    , const point_quadtree::Node& root
    , const primitives::point_id_t global_swap_end
    , const primitives::length_t removed_length
    , const primitives::length_t added_length
    , std::vector<primitives::point_id_t>& starts
    , std::vector<primitives::point_id_t>& ends
    , std::vector<primitives::point_id_t>& removed_edges)
{
    // each point p in removed_edges represents the edge (p, next(p)).
    // here, starts.size() == ends.size(), and removed_edges.size() == ends.size()
    const auto prev {tour.prev(ends.back())};
    auto new_start {prev};
    auto new_removal {prev};
    const auto has_prev_edge
    {
        std::find(std::cbegin(removed_edges), std::cend(removed_edges), new_removal)
            == std::cend(removed_edges)
    };
    if (has_prev_edge)
    {
        if (search_neighbors(tour, root
            , new_start, new_removal
            , global_swap_end
            , removed_length, added_length
            , starts, ends, removed_edges))
        {
            return true;
        }
    }
    const auto next {tour.next(ends.back())};
    new_start = next;
    new_removal = ends.back();
    const auto has_next_edge
    {
        std::find(std::cbegin(removed_edges), std::cend(removed_edges), new_removal)
            == std::cend(removed_edges)
    };
    if (has_next_edge)
    {
        if (search_neighbors(tour, root
            , new_start, new_removal
            , global_swap_end
            , removed_length, added_length
            , starts, ends, removed_edges))
        {
            return true;
        }
    }
    return false;
}

std::array<std::vector<primitives::point_id_t>, 3> start_search(Tour& tour
    , const point_quadtree::Node& root
    , const primitives::point_id_t swap_start
    , const primitives::point_id_t removed_edge
    , const primitives::point_id_t swap_end)
{
    const auto removed_length {tour.length(removed_edge)};
    std::vector<primitives::point_id_t> starts {swap_start};
    std::vector<primitives::point_id_t> removed_edges {removed_edge};
    std::vector<primitives::point_id_t> points;
    root.get_points(swap_start, tour.search_box(swap_start, removed_length + 1), points);
    for (auto p : points)
    {
        if (p == swap_start or p == tour.prev(swap_start) or p == tour.next(swap_start))
        {
            continue;
        }
        const auto added_length {tour.length(swap_start, p)};
        const auto gain {removed_length > added_length};
        if (not gain)
        {
            continue;
        }
        std::vector<primitives::point_id_t> ends {p};
        if (search_both_sides(tour, root
            , swap_end
            , removed_length
            , added_length
            , starts, ends, removed_edges))
        {
            return {starts, ends, removed_edges};
        }
    }
    return {};
}

std::array<std::vector<primitives::point_id_t>, 3> find_swap(Tour& tour
    , const point_quadtree::Node& root)
{
    constexpr primitives::point_id_t start {0};
    primitives::point_id_t i {start};
    do
    {
        const auto prev_result
        {
            start_search(tour, root
                , i
                , tour.prev(i)
                , tour.prev(i))
        };
        if (prev_result[0].size() > 0)
        {
            return prev_result;
        }
        const auto next_result
        {
            start_search(tour, root
                , i
                , i
                , tour.next(i))
        };
        if (next_result[0].size() > 0)
        {
            return next_result;
        }
        i = tour.next(i);
    } while (i != start);
    return {};
}

} // namespace multicycle
