#include "MaskedFeasibleFinder.h"


bool MaskedFeasibleFinder::find_best()
{
    reset_search();
    constexpr primitives::point_id_t start {0};
    primitives::point_id_t i {start};
    do
    {
        if (m_active.find(i) != std::cend(m_active))
        {
            m_swap_end = m_tour.prev(i);
            start_search(i, m_swap_end);
            if (found_improvement())
            {
                return true;
            }
            m_swap_end = m_tour.next(i);
            start_search(i, i);
            if (found_improvement())
            {
                return true;
            }
        }
        i = m_tour.next(i);
    } while (i != start);
    return false;
}
