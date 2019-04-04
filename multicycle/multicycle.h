#pragma once

#include "break_detection/break_detection.h"
#include "simple_merge/simple_merge.h"
#include <Tour.h>
#include <constants.h>
#include <point_quadtree/Node.h>
#include <primitives.h>

#include <algorithm> // find
#include <set>

namespace multicycle {

// Returns true if "swap" is improving.
bool search_neighbors(Tour& tour
    , const point_quadtree::Node& root
    , const primitives::point_id_t new_start
    , const primitives::point_id_t new_removal
    , const primitives::point_id_t global_swap_end
    , const primitives::length_t removed_length
    , const primitives::length_t added_length
    , std::vector<primitives::point_id_t>& starts
    , std::vector<primitives::point_id_t>& ends
    , std::vector<primitives::point_id_t>& removed_edges);

// Returns true if "swap" is improving.
bool search_both_sides(Tour& tour
    , const point_quadtree::Node& root
    , const primitives::point_id_t global_swap_end
    , const primitives::length_t removed_length
    , const primitives::length_t added_length
    , std::vector<primitives::point_id_t>& starts
    , std::vector<primitives::point_id_t>& ends
    , std::vector<primitives::point_id_t>& removed_edges);

std::array<std::vector<primitives::point_id_t>, 3> start_search(Tour& tour
    , const point_quadtree::Node& root
    , const primitives::point_id_t swap_start
    , const primitives::point_id_t removed_edge
    , const primitives::point_id_t swap_end);

std::array<std::vector<primitives::point_id_t>, 3> find_swap(Tour& tour
    , const point_quadtree::Node& root);

inline bool test_nonfeasible(const point_quadtree::Node& root
    , Tour& tour
    , const std::vector<primitives::point_id_t>& starts
    , const std::vector<primitives::point_id_t>& ends
    , const std::vector<primitives::point_id_t>& removed_edges)
{
    auto test_tour {tour};
    test_tour.multicycle_swap(starts, ends, removed_edges);
    while (test_tour.split())
    {
        // std::cout << "tour is split" << std::endl;
        simple_merge::merge_once(root, test_tour);
    }
    return test_tour.length() < tour.length();
}

inline void optimize(const point_quadtree::Node& root, Tour& tour)
{
    const auto start_length {tour.length()};
    bool improved {false};
    do
    {
        improved = false;

        // std::cout << "start multi search" << std::endl;
        const auto feasible_swap {multicycle::find_swap(tour, root)};
        const auto& starts {feasible_swap[0]};
        const auto& ends {feasible_swap[1]};
        const auto& removed_edges {feasible_swap[2]};

        /*
        int i {0};
        for (auto p : tour.order())
        {
            std::cout << i << ": " << p << std::endl;
            ++i;
        }
        std::cout << std::endl;
        for (size_t i {0}; i < starts.size(); ++i)
        {
            std::cout << starts[i] << ", " << ends[i]
                << ", " << removed_edges[i] << std::endl;
        }
        */

        // std::cout << "start multi swap" << std::endl;
        if (starts.size() > 0)
        {
            auto test_tour {tour};
            test_tour.multicycle_swap(starts, ends, removed_edges);
            while (test_tour.split())
            {
                // std::cout << "tour is split" << std::endl;
                simple_merge::merge_once(root, test_tour);
            }
            if (test_tour.length() < tour.length())
            {
                tour = test_tour;
            }
        }

        if (not multicycle::break_detection::single_cycle(tour))
        {
            std::cout << "not a single cycle" << std::endl;
            std::abort();
        }

        // std::cout << "finished iteration" << std::endl;
    } while (improved);
    if (tour.length() < start_length)
    {
        std::cout << "updated best length: " << tour.length() << std::endl;
    }
    // tour.validate();
}

} // namespace multicycle
