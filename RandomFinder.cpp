#include "RandomFinder.h"

std::vector<primitives::point_id_t>
RandomFinder::search_neighborhood(primitives::point_id_t p)
{
    m_max_kdepth = std::max(m_max_kdepth, m_kmove.current_k());
    const auto search_radius = m_kmargin.total_margin + 1;
    // TODO: filter out all points not within kmargin.
    auto points = m_root.get_points(p, m_box_maker(p, search_radius));
    if (m_kmove.current_k() < m_kmax or points.empty())
    {
        return points;
    }
    std::random_shuffle(std::begin(points), std::end(points));
    return {points[0]};
}

bool RandomFinder::final_new_edge() const
{
    return false;
}

