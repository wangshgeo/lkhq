#include "OptimalFinder.h"

std::vector<primitives::point_id_t>
OptimalFinder::search_neighborhood(primitives::point_id_t p) const
{
    const auto search_radius = m_kmargin.total_margin + 1;
    return m_root.get_points(p, m_box_maker(p, search_radius));
}

bool OptimalFinder::final_new_edge() const
{
    return m_kmove.current_k() == m_kmax;
}
