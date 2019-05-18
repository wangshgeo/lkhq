#pragma once

// From an initial tour that may be split into multiple cycles,
//  and an improvement or length margin,
//  computes a merged tour if it is an improvement (including margin).

#include "Tour.h"
#include "point_quadtree/Node.h"
#include "primitives.h"

class Merger
{
public:
    bool improved_tour() const { return m_improved_tour; }
    const Tour& best_tour() const { return m_best_tour; }
    void merge_once(const point_quadtree::Node& root
        , Tour& tour
        , primitives::length_t margin
        , bool negative_margin = false)
    {
        constexpr primitives::point_id_t start {0};
        const auto first_cycle = tour.cycle_id(start);
        for (primitives::point_id_t i {0}; i < tour.size(); ++i)
        {
            if (tour.cycle_id(i) != first_cycle)
            {
                continue;
            }
            auto max_radius = tour.length(i)
                + tour.max_outgroup_length();
            if (negative_margin)
            {
                if (margin / 2 >= max_radius)
                {
                    continue;
                }
                max_radius -= margin / 2;
            }
            else
            {
                max_radius += (margin + 1) / 2;
            }
            const auto points = root.get_points(i, tour.search_box(i, max_radius));
            for (auto p : points)
            {
                if (tour.prev(i) == p or tour.next(i) == p)
                {
                    continue;
                }
                if (tour.cycle_id(p) == tour.cycle_id(i))
                {
                    continue;
                }
                auto removed_length = tour.length(i) + tour.length(p);
                // TODO: factorize logic for both 2-opt possibilities.
                auto new_length = tour.length(i, p) + tour.length(tour.next(i), tour.next(p));
                auto effective_removed_length {removed_length};
                if (negative_margin)
                {
                    if (effective_removed_length <= margin)
                    {
                        continue;
                    }
                    effective_removed_length -= margin;
                }
                else
                {
                    effective_removed_length += margin;
                }
                if (new_length < effective_removed_length)
                {
                    auto new_margin = effective_removed_length - new_length;
                    auto new_tour = tour;
                    new_tour.multicycle_swap({i, tour.next(i)}, {p, tour.next(p)}, {i, p});
                    if (new_tour.min_cycle_size() < 2)
                    {
                        return;
                    }
                    if (new_tour.cycles() == 1)
                    {
                        m_improved_tour = true;
                        m_best_tour = new_tour;
                        return;
                    }
                    merge_once(root, new_tour, new_margin);
                    if (m_improved_tour)
                    {
                        return;
                    }
                }
                new_length = tour.length(i, tour.next(p)) + tour.length(tour.next(i), p);
                if (new_length < effective_removed_length)
                {
                    auto new_margin = effective_removed_length - new_length;
                    auto new_tour = tour;
                    new_tour.multicycle_swap({i, tour.next(i)}, {tour.next(p), p}, {i, p});
                    if (new_tour.min_cycle_size() < 2)
                    {
                        return;
                    }
                    if (new_tour.cycles() == 1)
                    {
                        m_improved_tour = true;
                        m_best_tour = new_tour;
                        return;
                    }
                    merge_once(root, new_tour, new_margin);
                    if (m_improved_tour)
                    {
                        return;
                    }
                }
            }
        }
    }

private:
    Tour m_best_tour;
    bool m_improved_tour {false};

};

