#include "Tour.h"

Tour::Tour(const point_quadtree::Domain* domain
    , const std::vector<primitives::point_id_t>& initial_tour
    , LengthMap* length_map)
: m_domain(domain)
, m_length_map(length_map)
, m_adjacents(initial_tour.size(), {constants::invalid_point, constants::invalid_point})
, m_next(initial_tour.size(), constants::invalid_point)
, m_sequence(initial_tour.size(), constants::invalid_point)
, m_cycle_id(initial_tour.size(), constants::invalid_cycle)
{
    reset_adjacencies(initial_tour);
    update_next();
}


void Tour::update_multicycle()
{
    std::fill(std::begin(m_cycle_id), std::end(m_cycle_id), constants::invalid_cycle);
    primitives::cycle_id_t current_cycle_id {0};
    primitives::point_id_t start {0};
    while (start != constants::invalid_point)
    {
        update_next(start, current_cycle_id);
        start = constants::invalid_point;
        for (primitives::point_id_t i {0}; i < size(); ++i)
        {
            if (m_cycle_id[i] == constants::invalid_cycle)
            {
                start = i;
                break;
            }
        }
        ++current_cycle_id;
    }
    m_cycle_end = current_cycle_id;
    constexpr primitives::point_id_t ingroup_point {0};
    const auto ingroup_id {m_cycle_id[ingroup_point]};
    m_max_outgroup_length = 0;
    for (primitives::point_id_t i {0}; i < size(); ++i)
    {
        if (m_cycle_id[i] != ingroup_id)
        {
            m_max_outgroup_length = std::max(m_max_outgroup_length, length(i));
        }
    }
}

void Tour::multicycle_swap(
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

void Tour::swap(const std::vector<primitives::point_id_t>& starts
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
    update_next();
}

primitives::point_id_t Tour::sequence(primitives::point_id_t i, primitives::point_id_t start) const
{
    auto start_sequence {m_sequence[start]};
    auto raw_sequence {m_sequence[i]};
    if (raw_sequence < start_sequence)
    {
        raw_sequence += m_sequence.size();
    }
    return raw_sequence - start_sequence;
}

Box Tour::search_box(primitives::point_id_t i, primitives::length_t radius) const
{
    auto x {m_length_map->x(i)};
    auto y {m_length_map->y(i)};
    Box box;
    box.xmin = x - radius;
    box.xmax = x + radius;
    box.ymin = y - radius;
    box.ymax = y + radius;
    return box;
}

Box Tour::search_box_next(primitives::point_id_t i) const
{
    return search_box(i, length(i) + 1);
}

Box Tour::search_box_prev(primitives::point_id_t i) const
{
    return search_box(i, prev_length(i) + 1);
}

void Tour::reset_adjacencies(const std::vector<primitives::point_id_t>& initial_tour)
{
    auto prev = initial_tour.back();
    for (auto p : initial_tour)
    {
        create_adjacency(p, prev);
        prev = p;
    }
}

primitives::point_id_t Tour::prev(primitives::point_id_t i) const
{
    const auto next {m_next[i]};
    if (m_adjacents[i][0] == next)
    {
        return m_adjacents[i][1];
    }
    else if (m_adjacents[i][1] == next)
    {
        return m_adjacents[i][0];
    }
    else
    {
        std::cout << __func__ << ": error: could not determine previous point." << std::endl;
        std::abort();
    }
}

primitives::length_t Tour::length() const
{
    primitives::length_t sum {0};
    for (primitives::point_id_t i {0}; i < m_next.size(); ++i)
    {
        sum += length(i);
    }
    return sum;
}

primitives::length_t Tour::prev_length(primitives::point_id_t i) const
{
    return m_length_map->length(i, prev(i));
}

primitives::length_t Tour::length(primitives::point_id_t i) const
{
    return m_length_map->length(i, m_next[i]);
}

std::vector<primitives::point_id_t> Tour::order() const
{
    constexpr primitives::point_id_t start {0};
    primitives::point_id_t current {start};
    std::vector<primitives::point_id_t> ordered_points;
    primitives::point_id_t count {0};
    do
    {
        ordered_points.push_back(current);
        current = m_next[current];
        if (count > m_next.size())
        {
            std::cout << __func__ << ": error: too many traversals." << std::endl;
            std::abort();
        }
        ++count;
    } while (current != start);
    return ordered_points;
}

void Tour::breaking_forward_swap(const std::vector<primitives::point_id_t>& swap)
{
    nonbreaking_forward_swap(swap, true);
    update_multicycle();
}

void Tour::nonbreaking_forward_swap(const std::vector<primitives::point_id_t>& swap, bool cyclic_first)
{
    // Use of prev() should precede use of break_adjacency().
    primitives::point_id_t last {prev(swap.front())};
    if (cyclic_first)
    {
        last = m_next[swap.front()];
    }
    std::vector<primitives::point_id_t> prevs;
    auto it = ++std::cbegin(swap);
    while (it != std::cend(swap))
    {
        prevs.push_back(prev(*it));
        ++it;
    }
    // for each point p in swap, edge (p, prev(p)) is deleted.
    for (size_t i {1}; i < swap.size(); ++i)
    {
        break_adjacency(prevs[i - 1], swap[i]);
    }
    if (cyclic_first)
    {
        break_adjacency(swap.front());
    }
    else
    {
        break_adjacency(swap.front(), last);
    }
    create_adjacency(swap[0], swap[1]);
    for (size_t i {2}; i < swap.size(); ++i)
    {
        create_adjacency(prevs[i - 2], swap[i]);
    }
    create_adjacency(prevs.back(), last);
    update_next();
}

void Tour::update_next(const primitives::point_id_t start
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
}

void Tour::update_next(const primitives::point_id_t start)
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
    } while (current != start); // tour cycle condition.
}

primitives::point_id_t Tour::get_other(primitives::point_id_t point, primitives::point_id_t adjacent) const
{
    const auto& a = m_adjacents[point];
    if (a.front() == adjacent)
    {
        return a.back();
    }
    else
    {
        return a.front();
    }
}

void Tour::create_adjacency(primitives::point_id_t point1, primitives::point_id_t point2)
{
    fill_adjacent(point1, point2);
    fill_adjacent(point2, point1);
}

void Tour::fill_adjacent(primitives::point_id_t point, primitives::point_id_t new_adjacent)
{
    if (m_adjacents[point].front() == constants::invalid_point)
    {
        m_adjacents[point].front() = new_adjacent;
    }
    else if (m_adjacents[point].back() == constants::invalid_point)
    {
        m_adjacents[point].back() = new_adjacent;
    }
    else
    {
        std::cout << __func__ << ": error: no available slot for new adjacent." << std::endl;
        std::cout << point << " -> " << new_adjacent << std::endl;
        std::abort();
    }
}

void Tour::break_adjacency(primitives::point_id_t i)
{
    break_adjacency(i, m_next[i]);
}

void Tour::break_adjacency(primitives::point_id_t point1, primitives::point_id_t point2)
{
    vacate_adjacent_slot(point1, point2, 0);
    vacate_adjacent_slot(point1, point2, 1);
    vacate_adjacent_slot(point2, point1, 0);
    vacate_adjacent_slot(point2, point1, 1);
}

void Tour::vacate_adjacent_slot(primitives::point_id_t point, primitives::point_id_t adjacent, int slot)
{
    if (m_adjacents[point][slot] == adjacent)
    {
        m_adjacents[point][slot] = constants::invalid_point;
    }
}

void Tour::validate() const
{
    constexpr primitives::point_id_t start {0};
    primitives::point_id_t current {start};
    size_t visited {0};
    do
    {
        ++visited;
        if (visited > m_next.size())
        {
            std::cout << __func__ << ": error: invalid tour." << std::endl;
            std::abort();
        }
        current = m_next[current];
    } while(current != start);
    if (visited != m_next.size())
    {
        std::cout << __func__ << ": error: invalid tour." << std::endl;
        std::abort();
    }
    std::cout << __func__ << ": success: tour valid." << std::endl;
}

