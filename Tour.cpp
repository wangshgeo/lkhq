#include "Tour.h"

Tour::Tour(const point_quadtree::Domain* domain
    , const std::vector<primitives::point_id_t>& initial_tour)
: m_domain(domain)
, m_adjacents(initial_tour.size(), {constants::invalid_point, constants::invalid_point})
, m_next(initial_tour.size(), constants::invalid_point)
, m_sequence(initial_tour.size(), constants::invalid_point)
, m_box_maker(domain->x(), domain->y())
, m_length_calculator(domain->x(), domain->y())
{
    reset_adjacencies(initial_tour);
    update_next();
}

void Tour::swap(const KMove& kmove)
{
    swap(kmove.starts, kmove.ends, kmove.removes);
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

primitives::sequence_t Tour::sequence(primitives::point_id_t i, primitives::point_id_t start) const
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
    return m_box_maker(i, radius);
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
    return m_length_calculator(i, prev(i));
}

primitives::length_t Tour::length(primitives::point_id_t i) const
{
    return m_length_calculator(i, m_next[i]);
}

primitives::length_t Tour::length(primitives::point_id_t i, primitives::point_id_t j) const
{
    return m_length_calculator(i, j);
}

void Tour::update_next(const primitives::point_id_t start)
{
    primitives::point_id_t current {start};
    m_next[current] = m_adjacents[current].front();
    primitives::point_id_t sequence {0};
    m_order.clear();
    m_order.reserve(m_next.size());
    do
    {
        auto prev = current;
        m_sequence[current] = sequence++;
        m_order.push_back(current);
        current = m_next[current];
        m_next[current] = get_other(current, prev);
    } while (current != start); // tour cycle condition.
    if (m_order.size() != m_next.size()) {
        throw std::logic_error("m_order was not build up properly.");
    }
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
        throw std::logic_error("invalid tour.");
    }
}

