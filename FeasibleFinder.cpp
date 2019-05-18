#include "FeasibleFinder.h"

bool FeasibleFinder::find_best()
{
    reset_search();
    constexpr primitives::point_id_t start {0};
    primitives::point_id_t i {start};
    do
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
        i = m_tour.next(i);
    } while (i != start);
    return false;
}

void FeasibleFinder::start_search(const primitives::point_id_t swap_start
    , const primitives::point_id_t removed_edge)
{
    const auto remove {m_tour.length(removed_edge)};
    m_starts.push_back(swap_start);
    m_removes.push_back(removed_edge);
    const auto search_box
    {
        m_tour.search_box(swap_start, remove + 1)
    };
    // TODO: consider storing this neighborhood.
    const auto points = m_root.get_points(swap_start, search_box);
    m_point_sum += points.size();
    ++m_point_neighborhoods;
    for (auto p : points)
    {
        if (p == swap_start
            or p == m_tour.prev(swap_start)
            or p == m_tour.next(swap_start))
        {
            continue;
        }
        const auto add {m_tour.length(swap_start, p)};
        if (not gainful(add, remove))
        {
            continue;
        }
        m_ends.push_back(p);
        search_both_sides(remove, add);
        if (found_improvement())
        {
            return;
        }
        m_ends.pop_back();
    }
    m_starts.pop_back();
    m_removes.pop_back();
}

// TODO: rename delete_edge
void FeasibleFinder::search_both_sides(const primitives::length_t removed
    , const primitives::length_t added)
{
    // each point p in m_removes represents the edge (p, next(p)).
    // here, m_starts.size() == m_ends.size(), and m_removes.size() == m_ends.size()
    const auto prev {m_tour.prev(m_ends.back())};
    auto new_remove {prev};
    const auto has_prev_edge
    {
        std::find(std::cbegin(m_removes), std::cend(m_removes), new_remove)
            == std::cend(m_removes)
    };
    if (has_prev_edge)
    {
        const auto new_start {prev};
        search_neighbors(new_start, new_remove, removed, added);
        if (found_improvement())
        {
            return;
        }
    }
    new_remove = m_ends.back();
    const auto has_next_edge
    {
        std::find(std::cbegin(m_removes), std::cend(m_removes), new_remove)
            == std::cend(m_removes)
    };
    if (has_next_edge)
    {
        const auto next {m_tour.next(m_ends.back())};
        const auto new_start {next};
        search_neighbors(new_start, new_remove, removed, added);
        if (found_improvement())
        {
            return;
        }
    }
}

// TODO: rename add_edge
void FeasibleFinder::search_neighbors(const primitives::point_id_t new_start
    , const primitives::point_id_t new_remove
    , const primitives::length_t removed
    , const primitives::length_t added)
{
    const auto remove {m_tour.length(new_remove)};
    // first check if can close.
    const auto closing_length {m_tour.length(new_start, m_swap_end)};
    const auto total_closing_add {closing_length + added};
    const auto total_remove {removed + remove};
    const bool improving {total_remove > total_closing_add};
    ++m_comparisons;
    if (improving)
    {
        if (new_start != m_tour.prev(m_swap_end) and new_start != m_tour.next(m_swap_end))
        {
            m_starts.push_back(new_start);
            m_ends.push_back(m_swap_end);
            m_removes.push_back(new_remove);
            const auto improvement {total_remove - total_closing_add};
            if (cycle_check::feasible(m_tour, m_starts, m_ends, m_removes))
            {
                //m_tour.print();
                //print_move();
                check_best(improvement);
                return;
            }
            else if (false) // TODO: non-sequential moves are a bit inefficient currently.
            {
                auto test_tour = m_tour;
                test_tour.multicycle_swap(m_starts, m_ends, m_removes);
                if (test_tour.cycles() == 1)
                {
                    for (auto o : m_tour.order())
                    {
                        std::cout << o << std::endl;
                    }
                    std::cout << __func__ << ": error: non-feasible swap is actually feasible" << std::endl;
                    print_move();
                    cycle_check::feasible(m_tour, m_starts, m_ends, m_removes);
                    std::abort();
                }
                else
                {
                    //std::cout << "legit non-feasible" << std::endl;
                }
            }
            m_starts.pop_back();
            m_ends.pop_back();
            m_removes.pop_back();
        }
    }
    if (m_starts.size() >= m_kmax - 1)
    {
        return;
    }

    const auto margin {total_remove - added};
    const auto search_box
    {
        m_tour.search_box(new_start, margin + m_tour.length(new_remove) + 1)
    };
    const auto points = m_root.get_points(new_start, search_box);
    m_point_sum += points.size();
    ++m_point_neighborhoods;
    for (auto p : points)
    {
        // check easy exclusion cases.
        const bool closing {p == m_swap_end}; // closing should already have been checked.

        const bool neighboring {p == m_tour.next(new_start) or p == m_tour.prev(new_start)};
        const bool self {p == new_start};
        const bool backtrack {p == m_starts.back()};
        if (backtrack or self or closing or neighboring)
        {
            continue;
        }

        // check if worth considering.
        const auto add {m_tour.length(new_start, p)};
        if (not gainful(add, margin))
        {
            continue;
        }

        // check if repeating move.
        const bool has_start
        {
            std::find(std::cbegin(m_starts), std::cend(m_starts), new_start)
                != std::cend(m_starts)
        };
        const bool has_end
        {
            std::find(std::cbegin(m_ends), std::cend(m_ends), p) != std::cend(m_ends)
        };
        if (has_start and has_end)
        {
            continue;
        }

        m_starts.push_back(new_start);
        m_ends.push_back(p);
        m_removes.push_back(new_remove);
        search_both_sides(removed + remove, added + add);
        if (found_improvement())
        {
            return;
        }
        m_starts.pop_back();
        m_ends.pop_back();
        m_removes.pop_back();
    }
}

const std::vector<primitives::point_id_t>&
    FeasibleFinder::get_start_points(primitives::point_id_t i, primitives::length_t radius) const
{
    if (m_start_neighborhoods[i].first.radius == radius)
    {
        return m_start_neighborhoods[i].first.near_points;
    }
    else if (m_start_neighborhoods[i].second.radius == radius)
    {
        return m_start_neighborhoods[i].second.near_points;
    }
    throw std::logic_error("could not find start neighborhood.");
}

