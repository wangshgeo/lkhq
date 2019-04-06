#pragma once

#include "LengthMap.h"
#include "constants.h"
#include "point_quadtree/Box.h"
#include "point_quadtree/Domain.h"
#include "multicycle/simple_merge/Swap.h"
#include "primitives.h"

#include <algorithm> // fill
#include <array>
#include <cstdlib> // abort
#include <iostream>
#include <vector>

class Tour
{
    using Adjacents = std::array<primitives::point_id_t, 2>;
public:
    Tour(const point_quadtree::Domain* domain
        , const std::vector<primitives::point_id_t>& initial_tour
        , LengthMap*);

    void nonbreaking_forward_swap(const std::vector<primitives::point_id_t>& swap, bool cyclic_first);
    void breaking_forward_swap(const std::vector<primitives::point_id_t>& swap);
    void swap(const std::vector<primitives::point_id_t>& starts
        , const std::vector<primitives::point_id_t>& ends
        , const std::vector<primitives::point_id_t>& removed_edges);
    void multicycle_swap(
        const std::vector<primitives::point_id_t>& starts
        , const std::vector<primitives::point_id_t>& ends
        , const std::vector<primitives::point_id_t>& removed_edges);
    void update_multicycle();

    primitives::point_id_t next(primitives::point_id_t i) const { return m_next[i]; }
    primitives::point_id_t prev(primitives::point_id_t i) const;
    std::vector<primitives::point_id_t> order() const;
    primitives::point_id_t size() const { return m_next.size(); }
    primitives::cycle_id_t cycle_id(primitives::point_id_t i) const { return m_cycle_id[i]; }
    bool split() const { return m_cycle_end > 1; }
    primitives::length_t max_outgroup_length() const { return m_max_outgroup_length; }

    primitives::point_id_t sequence(primitives::point_id_t i, primitives::point_id_t start) const;

    primitives::space_t x(primitives::point_id_t i) const { return m_length_map->x(i); }
    primitives::space_t y(primitives::point_id_t i) const { return m_length_map->y(i); }

    primitives::length_t length() const;
    primitives::length_t length(primitives::point_id_t i) const;
    primitives::length_t prev_length(primitives::point_id_t i) const;
    primitives::length_t length(primitives::point_id_t i, primitives::point_id_t j)
    {
        return m_length_map->length(i, j);
    }

    const LengthMap& length_map() const { return *m_length_map; }
    LengthMap* length_map() { return m_length_map; }
    const point_quadtree::Domain* domain() const { return m_domain; }

    Box search_box_next(primitives::point_id_t i) const;
    Box search_box_prev(primitives::point_id_t i) const;
    Box search_box(primitives::point_id_t i, primitives::length_t radius) const;

    void validate(bool suppress_success = false) const;
    bool async_update() const { return m_async_update; }
    void async_update(bool val) { m_async_update = val; }

private:
    const point_quadtree::Domain* m_domain {nullptr};
    LengthMap* m_length_map {nullptr};
    std::vector<Adjacents> m_adjacents;
    std::vector<primitives::point_id_t> m_next;
    std::vector<primitives::point_id_t> m_sequence;
    std::vector<primitives::cycle_id_t> m_cycle_id;
    primitives::cycle_id_t m_cycle_end {1}; // one-past-the-last cycle id.
    primitives::length_t m_max_outgroup_length {0};
    bool m_async_update {false};

    void reset_adjacencies(const std::vector<primitives::point_id_t>& initial_tour);
    void update_next(const primitives::point_id_t start = 0);
    void update_next(const primitives::point_id_t start
        , const primitives::cycle_id_t cycle_id);

    primitives::point_id_t get_other(primitives::point_id_t point, primitives::point_id_t adjacent) const;
    void create_adjacency(primitives::point_id_t point1, primitives::point_id_t point2);
    void fill_adjacent(primitives::point_id_t point, primitives::point_id_t new_adjacent);
    void break_adjacency(primitives::point_id_t i);
    void break_adjacency(primitives::point_id_t point1, primitives::point_id_t point2);
    void vacate_adjacent_slot(primitives::point_id_t point, primitives::point_id_t adjacent, int slot);
};

