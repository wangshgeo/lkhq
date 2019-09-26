#include "MulticycleTour.h"

void MulticycleTour::multicycle_swap(
    const std::vector<primitives::point_id_t>& starts
    , const std::vector<primitives::point_id_t>& ends
    , const std::vector<primitives::point_id_t>& removed_edges)
{
    for (auto p : removed_edges)
    {
        break_adjacency(p);
    }
    for (size_t i {0}; i < starts.size(); ++i)
    {
        create_adjacency(starts[i], ends[i]);
    }
    update_multicycle();
}

void MulticycleTour::update_multicycle()
{
    std::fill(std::begin(m_cycle_id), std::end(m_cycle_id), constants::invalid_cycle);
    constexpr primitives::point_id_t first_group_start {0};
    primitives::point_id_t cycle_start {first_group_start};
    m_cycle_end = 0;
    m_min_cycle_size = std::numeric_limits<size_t>::max();
    while (cycle_start != constants::invalid_point)
    {
        auto cycle_size = update_next(cycle_start, m_cycle_end);
        m_min_cycle_size = std::min(cycle_size, m_min_cycle_size);
        //std::cout << "cycle size: " << cycle_size << std::endl;
        cycle_start = constants::invalid_point;
        for (primitives::point_id_t i {0}; i < size(); ++i)
        {
            if (m_cycle_id[i] == constants::invalid_cycle)
            {
                cycle_start = i;
                break;
            }
        }
        ++m_cycle_end;
    }
}

size_t MulticycleTour::update_next(const primitives::point_id_t start
    , const primitives::cycle_id_t cycle_id)
{
    primitives::point_id_t current {start};
    m_next[current] = m_adjacents[current].front();
    primitives::point_id_t sequence {0};
    do
    {
        auto prev = current;
        m_sequence[current] = sequence++;
        current = m_next[current];
        m_next[current] = get_other(current, prev);
        m_cycle_id[current] = cycle_id;
        if (sequence > size())
        {
            std::cout << __func__ << ": error: sequence is higher than total number of points." << std::endl;
            std::abort();
        }
    } while (current != start); // tour cycle condition.
    return sequence;
}

