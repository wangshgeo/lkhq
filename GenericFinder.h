#pragma once

#include "BoxMaker.h"
#include "Config.h"
#include "KMargin.h"
#include "KMove.h"
#include "Tour.h"
#include "constants.h"
#include "cycle_check.h"
#include "point_quadtree/Node.h"
#include "primitives.h"

#include <array>
#include <optional>

template <typename Derived>
class GenericFinder
{
public:
    GenericFinder(const Config& config, const point_quadtree::Node& root, Tour& tour)
        : m_root(root)
        , m_tour(tour)
        , m_box_maker(tour.x(), tour.y())
        , m_length_calculator(tour.x(), tour.y())
        , m_kmax(config.get<size_t>("kmax", m_kmax)) {}

    std::optional<KMove> find_best();
    std::optional<KMove> find_best(std::nullopt_t) { return find_best(); }

    auto& tour() { return m_tour; }

protected:
    const point_quadtree::Node& m_root;
    Tour& m_tour;
    const BoxMaker m_box_maker;
    LengthCalculator m_length_calculator;
    size_t m_kmax {3};

    KMove m_kmove;
    primitives::point_id_t m_swap_end {constants::invalid_point};
    bool m_stop {false};

    KMargin m_kmargin;

    void search(primitives::point_id_t i);
    void delete_both_edges();
    void try_nearby_points();

    void reset_search();

    primitives::length_t length(primitives::point_id_t edge_start) const;
    primitives::length_t length(primitives::point_id_t a, primitives::point_id_t b) const;

    void final_move_check();
    bool final_new_edge() const;
    void final_stats() const {}

    std::vector<primitives::point_id_t> search_neighborhood(primitives::point_id_t p) const;

private:
    auto* derived() { return static_cast<Derived*>(this); }
    const auto* derived() const { return static_cast<const Derived*>(this); }

};

template <typename Derived>
void GenericFinder<Derived>::final_move_check()
{
    if (cycle_check::feasible(m_tour, m_kmove))
    {
        m_stop = true;
    }
}

template <typename Derived>
bool GenericFinder<Derived>::final_new_edge() const
{
    return m_kmove.current_k() == m_kmax;
}

template <typename Derived>
std::vector<primitives::point_id_t>
GenericFinder<Derived>::search_neighborhood(primitives::point_id_t p) const
{
    const auto search_radius = m_kmargin.total_margin + 1;
    return m_root.get_points(p, m_box_maker(p, search_radius));
}

template <typename Derived>
std::optional<KMove> GenericFinder<Derived>::find_best()
{
    reset_search();
    for (primitives::point_id_t i {0}; i < m_tour.size(); ++i)
    {
        search(i);
        if (m_stop)
        {
            return m_kmove;
        }
    }
    derived()->final_stats();
    return std::nullopt;
}

template <typename Derived>
void GenericFinder<Derived>::search(primitives::point_id_t i)
{
    m_kmove.starts.push_back(i);

    const std::array<primitives::point_id_t, 2> back_pair {m_tour.prev(i), m_tour.prev(i)};
    const std::array<primitives::point_id_t, 2> front_pair {i, m_tour.next(i)};
    for(auto [edge, swap_end] : {back_pair, front_pair})
    {
        m_kmove.removes.push_back(edge);
        m_kmargin.increase(length(edge));
        m_swap_end = swap_end;
        try_nearby_points();
        if (m_stop)
        {
            return;
        }
        m_kmove.removes.pop_back();
        m_kmargin.pop_increase();
    }

    m_kmove.starts.pop_back();
}

template <typename Derived>
void GenericFinder<Derived>::try_nearby_points()
{
    const auto start = m_kmove.starts.back();
    for (auto p : derived()->search_neighborhood(start))
    {
        // check easy exclusion cases.
        const bool old_edge {p == m_tour.next(start) or p == m_tour.prev(start)};
        const bool self {p == start};
        const bool backtrack {(not m_kmove.ends.empty()) and p == m_kmove.ends.back()};
        if (backtrack or self or old_edge)
        {
            continue;
        }

        // check if worth considering.
        if (m_kmargin.decrease(length(start, p)))
        {
            if (m_kmove.endable(p))
            {
                m_kmove.ends.push_back(p);
                // check if closing swap.
                if (p == m_swap_end)
                {
                    derived()->final_move_check();
                    if (m_stop)
                    {
                        return;
                    }
                }
                delete_both_edges();
                if (m_stop)
                {
                    return;
                }
                m_kmove.ends.pop_back();
            }
            m_kmargin.pop_decrease();
        }
    }
}

template <typename Derived>
void GenericFinder<Derived>::delete_both_edges()
{
    const auto i = m_kmove.ends.back();
    const std::array<primitives::point_id_t, 2> back_pair {m_tour.prev(i), m_tour.prev(i)};
    const std::array<primitives::point_id_t, 2> front_pair {i, m_tour.next(i)};
    for(auto [edge, start] : {back_pair, front_pair})
    {
        if (not m_kmove.removable(edge) or not m_kmove.startable(start))
        {
            continue;
        }
        m_kmove.starts.push_back(start);
        m_kmove.removes.push_back(edge);
        m_kmargin.increase(length(edge));
        if (derived()->final_new_edge())
        {
            if (m_kmargin.decrease(length(start, m_swap_end)))
            {
                m_kmove.ends.push_back(m_swap_end);
                derived()->final_move_check();
                if (m_stop)
                {
                    return;
                }
                m_kmove.ends.pop_back();
                m_kmargin.pop_decrease();
            }
        }
        else
        {
            try_nearby_points();
            if (m_stop)
            {
                return;
            }
        }
        m_kmove.starts.pop_back();
        m_kmove.removes.pop_back();
        m_kmargin.pop_increase();
    }
}

template <typename Derived>
void GenericFinder<Derived>::reset_search()
{
    m_kmove.clear();
    m_kmargin.clear();
    m_swap_end = constants::invalid_point;
    m_stop = false;
}

template <typename Derived>
primitives::length_t GenericFinder<Derived>::length(primitives::point_id_t a, primitives::point_id_t b) const
{
    return m_length_calculator(a, b);
}

template <typename Derived>
primitives::length_t GenericFinder<Derived>::length(primitives::point_id_t edge_start) const
{
    return m_length_calculator(edge_start, m_tour.next(edge_start));
}
