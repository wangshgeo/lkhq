#pragma once

#include "Box.h"
#include "BoxMaker.h"
#include "LengthMap.h"
#include "constants.h"
#include "multicycle/simple_merge/Swap.h"
#include "point_quadtree/Domain.h"
#include "point_quadtree/Node.h"
#include "primitives.h"

#include <algorithm> // fill
#include <array>
#include <cstdlib> // abort
#include <iostream>
#include <random> // sample
#include <vector>

class Tour
{
    using Adjacents = std::array<primitives::point_id_t, 2>;
public:
    Tour() = default;
    Tour(const point_quadtree::Domain* domain
        , const std::vector<primitives::point_id_t>& initial_tour
        , LengthMap*);

    void double_bridge_perturbation();

    void nonbreaking_forward_swap(const std::vector<primitives::point_id_t>& swap, bool cyclic_first);
    void breaking_forward_swap(const std::vector<primitives::point_id_t>& swap);
    template <typename PointContainer = std::vector<primitives::point_id_t>>
    void swap(const PointContainer& starts, const PointContainer& ends, const PointContainer& removed_edges);
    void multicycle_swap(
        const std::vector<primitives::point_id_t>& starts
        , const std::vector<primitives::point_id_t>& ends
        , const std::vector<primitives::point_id_t>& removed_edges);
    void update_multicycle();
    size_t min_cycle_size() const { return m_min_cycle_size; }

    auto next(primitives::point_id_t i) const { return m_next[i]; }
    primitives::point_id_t prev(primitives::point_id_t i) const;
    std::vector<primitives::point_id_t> order() const;
    size_t size() const { return m_next.size(); }
    auto cycle_id(primitives::point_id_t i) const { return m_cycle_id[i]; }
    bool split() const { return m_cycle_end > 1; }
    auto cycles() const { return m_cycle_end; }
    auto max_outgroup_length() const { return m_max_outgroup_length; }

    primitives::point_id_t sequence(primitives::point_id_t i, primitives::point_id_t start) const;

    const auto& x() const { return m_length_map->x(); }
    const auto& y() const { return m_length_map->y(); }
    auto x(primitives::point_id_t i) const { return m_length_map->x(i); }
    auto y(primitives::point_id_t i) const { return m_length_map->y(i); }

    primitives::length_t length() const;
    primitives::length_t length(primitives::point_id_t i) const;
    primitives::length_t prev_length(primitives::point_id_t i) const;
    primitives::length_t length(primitives::point_id_t i, primitives::point_id_t j)
    {
        return m_length_map->length(i, j);
    }

    const auto& length_map() const { return *m_length_map; }
    auto length_map() { return m_length_map; }
    auto domain() const { return m_domain; }

    Box search_box_next(primitives::point_id_t i) const;
    Box search_box_prev(primitives::point_id_t i) const;
    Box search_box(primitives::point_id_t i, primitives::length_t radius) const;

    void validate(bool suppress_success = false) const;

    void print_first_cycle() const
    {
        constexpr primitives::point_id_t start {0};
        primitives::point_id_t current {start};
        std::vector<bool> visited(size(), false);
        size_t counter {0};
        do
        {
            //std::cout << current << std::endl;
            current = m_next[current];
            visited[current] = true;
            ++counter;
        } while (current != start);
        std::cout << __func__ << ": first cycle size: " << counter << std::endl;
        //std::cout << "outgroup:" << std::endl;
        for (size_t i {0}; i < size(); ++i)
        {
            if (not visited[i])
            {
                //std::cout << i << std::endl;
            }
        }
    }

    void print() const
    {
        constexpr primitives::point_id_t start {0};
        primitives::point_id_t current {start};
        do
        {
            std::cout << current << std::endl;
            current = m_next[current];
        } while (current != start);
    }

    std::vector<primitives::point_id_t> get_points(
        const point_quadtree::Node& root
        , primitives::point_id_t i
        , primitives::length_t radius) const
    {
        return root.get_points(i, search_box(i, radius));
    }

private:
    const point_quadtree::Domain* m_domain {nullptr};
    LengthMap* m_length_map {nullptr};
    std::vector<Adjacents> m_adjacents;
    std::vector<primitives::point_id_t> m_next;
    std::vector<primitives::point_id_t> m_sequence;
    std::vector<primitives::cycle_id_t> m_cycle_id;
    primitives::cycle_id_t m_cycle_end {1}; // one-past-the-last cycle id.
    primitives::length_t m_max_outgroup_length {0};
    size_t m_min_cycle_size {0};
    BoxMaker m_box_maker;

    void reset_adjacencies(const std::vector<primitives::point_id_t>& initial_tour);
    void update_next(const primitives::point_id_t start = 0);
    // returns size of cycle.
    size_t update_next(const primitives::point_id_t start
        , const primitives::cycle_id_t cycle_id);

    primitives::point_id_t get_other(primitives::point_id_t point, primitives::point_id_t adjacent) const;
    void create_adjacency(primitives::point_id_t point1, primitives::point_id_t point2);
    void fill_adjacent(primitives::point_id_t point, primitives::point_id_t new_adjacent);
    void break_adjacency(primitives::point_id_t i);
    void break_adjacency(primitives::point_id_t point1, primitives::point_id_t point2);
    void vacate_adjacent_slot(primitives::point_id_t point, primitives::point_id_t adjacent);
};

template <typename PointContainer>
void Tour::swap(const PointContainer& starts, const PointContainer& ends, const PointContainer& removed_edges)
{
    for (auto p : removed_edges)
    {
        break_adjacency(p);
    }
    for (size_t i {0}; i < starts.size(); ++i)
    {
        create_adjacency(starts[i], ends[i]);
    }
    update_next();
}

