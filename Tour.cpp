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
, m_box_maker(length_map->x(), length_map->y())
{
    reset_adjacencies(initial_tour);
    update_next();
}

void Tour::double_bridge_perturbation()
{
    constexpr size_t segments_to_replace {4};
    std::array<primitives::point_id_t, segments_to_replace> old_starts {{constants::invalid_point}};
    std::sample(std::cbegin(m_next), std::cend(m_next), std::begin(old_starts)
        , segments_to_replace
        , std::mt19937(std::random_device{}())); // TODO: ensure this is uniformly random.
    std::sort(std::begin(old_starts), std::end(old_starts)
        , [&m_sequence = m_sequence](primitives::point_id_t lhs, primitives::point_id_t rhs)
            {
                return m_sequence[lhs] < m_sequence[rhs];
            });
    std::array<primitives::point_id_t, segments_to_replace> ends
    {{
        next(old_starts[2])
        , next(old_starts[3])
        , next(old_starts[0])
        , next(old_starts[1])
    }};
    /*
    std::cout << "tour start:" << std::endl;
    print();
    std::cout << "tour end." << std::endl;
    std::cout << "old starts:";
    for (auto s : old_starts)
    {
        std::cout << " " << s;
    }
    std::cout << std::endl;
    std::cout << "ends:";
    for (auto s : ends)
    {
        std::cout << " " << s;
    }
    std::cout << std::endl;
    */
    swap(old_starts, ends, old_starts);
}

void Tour::update_multicycle()
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
    // TODO: merge this with previous traversal.
    const auto ingroup_id {m_cycle_id[first_group_start]};
    m_max_outgroup_length = 0;
    for (primitives::point_id_t i {0}; i < size(); ++i)
    {
        if (m_cycle_id[i] != ingroup_id)
        {
            m_max_outgroup_length = std::max(m_max_outgroup_length, length(i));
        }
    }
}

template <typename T>
void print_short_vec(const std::vector<T>& vec)
{
    for(auto v : vec)
    {
        std::cout << "\t" << v;
    }
    std::cout << std::endl;
}

void Tour::multicycle_swap(
    const std::vector<primitives::point_id_t>& starts
    , const std::vector<primitives::point_id_t>& ends
    , const std::vector<primitives::point_id_t>& removed_edges)
{
    /*
    for (auto o : order())
    {
        std::cout << o << std::endl;
    }
    std::cout << "starts:";
    print_short_vec(starts);
    std::cout << "ends:";
    print_short_vec(ends);
    std::cout << "removes:";
    print_short_vec(removed_edges);
    std::cout << __func__ << std::endl;
    */
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
        print_first_cycle();
        std::cout << "next, i, adjacents: "<< next
            << ", " << i
            << ", " << m_adjacents[i][0]
            << ", " << m_adjacents[i][1]
            << std::endl;
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
    primitives::point_id_t start {0};
    primitives::point_id_t current {start};
    std::vector<primitives::point_id_t> ordered_points;
    primitives::point_id_t count {0};
    std::vector<bool> visited(size(), false);
    while (count < size())
    {
        do
        {
            ordered_points.push_back(current);
            visited[current] = true;
            current = m_next[current];
            if (count > m_next.size())
            {
                std::cout << __func__ << ": error: too many traversals." << std::endl;
                std::abort();
            }
            ++count;
        } while (current != start);
        for (primitives::point_id_t i {0}; i < size(); ++i)
        {
            if (not visited[i])
            {
                current = start = i;
                break;
            }
        }
    }
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

size_t Tour::update_next(const primitives::point_id_t start
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
    vacate_adjacent_slot(point1, point2);
    vacate_adjacent_slot(point2, point1);
}

void Tour::vacate_adjacent_slot(primitives::point_id_t point, primitives::point_id_t adjacent)
{
    if (m_adjacents[point][0] == adjacent)
    {
        m_adjacents[point][0] = constants::invalid_point;
    }
    else if (m_adjacents[point][1] == adjacent)
    {
        m_adjacents[point][1] = constants::invalid_point;
    }
}

void Tour::validate(bool suppress_success) const
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
    if (not suppress_success)
    {
        std::cout << __func__ << ": success: tour valid." << std::endl;
    }
}

