#include "Finder.h"

namespace perturbation {

Finder::Finder(const point_quadtree::Node& root, Tour& tour
    , primitives::length_t target_cost)
    : m_root(root), m_tour(tour), m_target_cost(target_cost)
    // , m_gain_relaxation(target_cost)
{}

// returns true next cost was found.
bool Finder::search()
{
    if (m_next_cost != constants::max_length)
    {
        std::cout << "changing target cost from " << m_target_cost
            << " to " << m_next_cost << std::endl;
        m_target_cost = m_next_cost;
    }
    else
    {
        std::cout << "target cost: " << m_target_cost << std::endl;
    }
    m_swaps.clear();
    m_cyclic_firsts.clear();
    m_next_cost = constants::max_length;
    find_forward_swap();
    find_forward_swap_ab();
    std::cout << "found " << m_swaps.size()
        << " perturbations with cost " << m_target_cost
        << std::endl;
    if (m_next_cost == constants::max_length)
    {
        m_target_cost = 0;
    }
    return m_next_cost != constants::max_length;
}

void Finder::find_forward_swap(
    const primitives::point_id_t global_swap_start
    , const primitives::point_id_t global_swap_end
    , const primitives::point_id_t edge_start
    , const primitives::length_t removed_length
    , const primitives::length_t added_length
    , std::vector<primitives::point_id_t>& swap
    , bool restrict_even_removals)
{
    std::vector<primitives::point_id_t> points;
    m_root.get_points(edge_start, m_tour.search_box_next(edge_start), points);
    const auto minimum_sequence {m_tour.sequence(edge_start, global_swap_start) + 2};
    for (auto p : points)
    {
        if (m_tour.sequence(p, global_swap_start) < minimum_sequence)
        {
            continue;
        }
        auto addition {m_tour.length(p, edge_start)};
        auto deletion {m_tour.length(edge_start)};
        if (addition >= deletion + m_gain_relaxation)
        {
            continue;
        }
        swap.push_back(p);
        const auto new_start {m_tour.prev(p)};
        const auto closing_length {m_tour.length(global_swap_end, new_start)};
        const auto total_added_length {closing_length + addition + added_length};
        const auto total_removed_length {removed_length + deletion};
        if (m_target_cost + total_removed_length < total_added_length)
        {
            m_next_cost = std::min(m_next_cost, total_added_length - total_removed_length);
            if (m_next_cost <= m_target_cost)
            {
                std::abort();
            }
        }
        if (m_target_cost + total_removed_length == total_added_length)
        {
            const bool odd_swap_size {(swap.size() & 1) == 1};
            const bool non_breaking {not restrict_even_removals or odd_swap_size};
            if (non_breaking)
            {
                //std::cout << "lateral: " << swap.size() << std::endl;
                m_swaps.push_back(swap);
                m_cyclic_firsts.push_back(restrict_even_removals);
            }
        }
        find_forward_swap(global_swap_start, global_swap_end
            , new_start
            , removed_length + deletion
            , added_length + addition
            , swap
            , restrict_even_removals);
        swap.pop_back();
    }
}

void Finder::find_forward_swap()
{
    // for each point p in swap vector, edge (p, prev(p)) is deleted.
    constexpr primitives::point_id_t start {0};
    primitives::point_id_t i {start};
    do
    {
        // If the "first" (direction of traversal) point of any edge is "a"
        //  and the second point is "b", then the first move can be made of either:
        // 1. first destroyed edge b to next destroyed edge b
        // 2. first destroyed edge a to next destroyed edge b
        // In option 2, even-numbered edge removals cannot result in a single cycle,
        //  e.g. 2-opt, 4-opt, 6-opt, etc. cannot be performed without splitting the
        //  tour into 2 cycles.
        // In a forward swap, all subsequent destroyed edges must be
        //  downstream / later in the tour, and all moves (except the first)
        //  must go from a to b (of the next destroyed edge).

        // option 1
        std::vector<primitives::point_id_t> points;
        m_root.get_points(i, m_tour.search_box_prev(i), points);
        const auto global_swap_end {m_tour.prev(i)};
        for (auto p : points)
        {
            if (p == i or p == m_tour.prev(i) or p == m_tour.next(i))
            {
                continue;
            }
            const auto addition {m_tour.length(p, i)};
            const auto deletion {m_tour.prev_length(i)};
            if (addition >= deletion + m_gain_relaxation)
            {
                continue;
            }
            const auto new_start {m_tour.prev(p)};
            const auto next_deletion {m_tour.prev_length(p)};
            const auto closing_length {m_tour.length(global_swap_end, new_start)};
            std::vector<primitives::point_id_t> swap {i, p};
            const auto total_added_length {closing_length + addition};
            const auto total_removed_length {deletion + next_deletion};
            if (total_added_length > m_target_cost + total_removed_length)
            {
                m_next_cost = std::min(m_next_cost, total_added_length - total_removed_length);
                if (m_next_cost <= m_target_cost)
                {
                    std::abort();
                }
            }
            if (total_added_length == m_target_cost + total_removed_length)
            {
                //std::cout << "lateral: " << swap.size() << std::endl;
                m_swaps.push_back(swap); // 2-opt.
                m_cyclic_firsts.push_back(false);
            }
            find_forward_swap(global_swap_end, global_swap_end
                , new_start
                , deletion
                , addition
                , swap
                , false);
        }
        i = m_tour.next(i);
    } while (i != start);
}

// For the first move, the first point in the first edge connects to the next point
//  (as opposed to the second point in the first edge).
// This means that the first move creates a cycle and cannot be closed
//  (e.g. a 2-opt cannot be performed).
void Finder::find_forward_swap_ab()
{
    // for each point p in swap vector, edge (p, prev(p)) is deleted.
    constexpr primitives::point_id_t start {0};
    primitives::point_id_t i {start};
    do
    {
        // If the "first" (direction of traversal) point of any edge is "a"
        //  and the second point is "b", then the first move can be made of either:
        // 1. first destroyed edge b to next destroyed edge b
        // 2. first destroyed edge a to next destroyed edge b
        // In option 2, even-numbered edge removals cannot result in a single cycle,
        //  e.g. 2-opt, 4-opt, 6-opt, etc. cannot be performed without splitting the
        //  tour into 2 cycles.
        // In a forward swap, all subsequent destroyed edges must be
        //  downstream / later in the tour, and all moves (except the first)
        //  must go from a to b (of the next destroyed edge).

        // option 2
        std::vector<primitives::point_id_t> points;
        m_root.get_points(i, m_tour.search_box_next(i), points);
        for (auto p : points)
        {
            if (p == i or p == m_tour.prev(i) or p == m_tour.next(i))
            {
                continue;
            }
            const auto addition {m_tour.length(p, i)};
            const auto deletion {m_tour.length(i)};
            if (addition >= deletion + m_gain_relaxation)
            {
                continue;
            }
            const auto new_start {m_tour.prev(p)};
            std::vector<primitives::point_id_t> swap {i, p};
            const auto global_swap_start {i};
            const auto global_swap_end {m_tour.next(i)};
            find_forward_swap(global_swap_start, global_swap_end
                , new_start
                , deletion
                , addition
                , swap
                , true);
        }
        i = m_tour.next(i);
    } while (i != start);
}

void Finder::test_perturbations()
{
    const auto start_length {m_tour.length()};
    std::cout << "start length: " << start_length << std::endl;
    for (size_t i {0}; i < m_swaps.size(); ++i)
    {
        auto new_tour {m_tour};
        new_tour.nonbreaking_forward_swap(m_swaps[i], m_cyclic_firsts[i]);
        forward::optimize(m_root, new_tour, true, false);
        const auto new_length {new_tour.length()};
        if (new_length < start_length)
        {
            std::cout << "updated tour length: " << new_length << std::endl;
            m_tour = new_tour;
        }
    }
    std::cout << "next cost: " << m_next_cost << std::endl;
}

} // namespace perturbation
