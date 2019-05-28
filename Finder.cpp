#include "Finder.h"

std::optional<KMove> Finder::find_best()
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
    return std::nullopt;
}

void Finder::search(primitives::point_id_t i)
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

void Finder::try_nearby_points()
{
    const auto start = m_kmove.starts.back();
    const auto search_radius = m_kmargin.total_margin + 1;
    const auto points = m_root.get_points(start, m_box_maker(start, search_radius));
    for (auto p : points)
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
                    if (cycle_check::feasible(m_tour, m_kmove))
                    {
                        m_kmove.validate();
                        m_stop = true;
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

void Finder::delete_both_edges()
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
        if (m_kmove.current_k() == m_kmax)
        {
            if (m_kmargin.decrease(length(start, m_swap_end)))
            {
                m_kmove.ends.push_back(m_swap_end);
                if (cycle_check::feasible(m_tour, m_kmove))
                {
                    m_kmove.validate();
                    m_stop = true;
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

void Finder::reset_search()
{
    m_kmove.clear();
    m_kmargin.clear();
    m_swap_end = constants::invalid_point;
    m_stop = false;
}

primitives::length_t Finder::length(primitives::point_id_t a, primitives::point_id_t b) const
{
    return m_tour.length(a, b);
}

primitives::length_t Finder::length(primitives::point_id_t a) const
{
    return m_tour.length(a);
}

