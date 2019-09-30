#include "hill_climber.hh"

void HillClimber::final_move_check() {
    if (cycle_check::feasible(*m_tour, m_kmove)) {
        m_stop = true;
    }
}

bool HillClimber::final_new_edge() const {
    return m_kmove.current_k() == m_kmax;
}

std::vector<primitives::point_id_t> HillClimber::search_neighborhood(primitives::point_id_t p) const {
    const auto search_radius = m_kmargin.total_margin + 1;
    return m_point_set.get_points(p, search_radius);
}

std::optional<KMove> HillClimber::find_best(const Tour &tour, size_t kmax) {
    m_tour = &tour;
    m_kmax = kmax;
    reset_search();
    for (primitives::point_id_t i {0}; i < size(); ++i) {
        search(i);
        if (m_stop) {
            return m_kmove;
        }
    }
    return std::nullopt;
}

void HillClimber::search(primitives::point_id_t i) {
    m_kmove.starts.push_back(i);

    const std::array<primitives::point_id_t, 2> back_pair {prev(i), prev(i)};
    const std::array<primitives::point_id_t, 2> front_pair {i, next(i)};
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

void HillClimber::try_nearby_points()
{
    const auto start = m_kmove.starts.back();
    for (auto p : search_neighborhood(start))
    {
        // check easy exclusion cases.
        const bool old_edge {p == next(start) or p == prev(start)};
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
                    final_move_check();
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

void HillClimber::delete_both_edges() {
    const auto i = m_kmove.ends.back();
    const std::array<primitives::point_id_t, 2> back_pair {prev(i), prev(i)};
    const std::array<primitives::point_id_t, 2> front_pair {i, next(i)};
    for(auto [edge, start] : {back_pair, front_pair})
    {
        if (not m_kmove.removable(edge) or not m_kmove.startable(start))
        {
            continue;
        }
        m_kmove.starts.push_back(start);
        m_kmove.removes.push_back(edge);
        m_kmargin.increase(length(edge));
        if (final_new_edge())
        {
            if (m_kmargin.decrease(length(start, m_swap_end)))
            {
                m_kmove.ends.push_back(m_swap_end);
                final_move_check();
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

void HillClimber::reset_search() {
    m_kmove.clear();
    m_kmargin.clear();
    m_swap_end = constants::invalid_point;
    m_stop = false;
}

primitives::length_t HillClimber::length(primitives::point_id_t a, primitives::point_id_t b) const {
    return m_point_set.length(a, b);
}

primitives::length_t HillClimber::length(primitives::point_id_t edge_start) const {
    return m_point_set.length(edge_start, next(edge_start));
}
