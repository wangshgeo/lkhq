#include "Finder.h"

bool Finder::find_best()
{
    reset_search();
    constexpr primitives::point_id_t start {0};
    primitives::point_id_t i {start};
    do
    {
        m_swap_end = m_tour.prev(i);
        start_search(i, m_swap_end);
        m_swap_end = m_tour.next(i);
        start_search(i, i);
        i = m_tour.next(i);
    } while (i != start);
    return m_best_improvement > 0;
}

void Finder::start_search(const primitives::point_id_t swap_start
    , const primitives::point_id_t removed_edge)
{
    if (m_first_improvement and m_best_improvement > 0)
    {
        return;
    }
    const auto remove {m_tour.length(removed_edge)};
    m_starts.push_back(swap_start);
    m_removes.push_back(removed_edge);
    std::vector<primitives::point_id_t> points;
    const auto search_box
    {
        m_tour.search_box(swap_start, remove + 1)
    };
    m_root.get_points(swap_start, search_box, points);
    for (auto p : points)
    {
        if (p == swap_start
            or p == m_tour.prev(swap_start)
            or p == m_tour.next(swap_start))
        {
            continue;
        }
        const auto add {m_tour.length(swap_start, p)};
        if (add > remove)
        {
            continue;
        }
        m_ends.push_back(p);
        search_both_sides(remove, add);
        m_ends.pop_back();
    }
    m_starts.pop_back();
    m_removes.pop_back();
}

void Finder::search_both_sides(const primitives::length_t removed
    , const primitives::length_t added)
{
    if (m_first_improvement and m_best_improvement > 0)
    {
        return;
    }
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
    }
}

void Finder::search_neighbors(const primitives::point_id_t new_start
    , const primitives::point_id_t new_remove
    , const primitives::length_t removed
    , const primitives::length_t added)
{
    if (m_first_improvement and m_best_improvement > 0)
    {
        return;
    }
    const auto remove {m_tour.length(new_remove)};
    // first check if can close.
    const auto closing_length {m_tour.length(new_start, m_swap_end)};
    const auto total_closing_add {closing_length + added};
    const auto total_remove {removed + remove};
    if (m_save_lateral_moves and total_remove == total_closing_add)
    {
        m_starts.push_back(new_start);
        m_ends.push_back(m_swap_end);
        m_removes.push_back(new_remove);
        if (feasible())
        {
            m_lateral_moves.push_back({m_starts, m_ends, m_removes});
        }
        m_starts.pop_back();
        m_ends.pop_back();
        m_removes.pop_back();
    }
    const bool improving {total_remove > total_closing_add};
    if (improving)
    {
        if (new_start != m_tour.prev(m_swap_end) and new_start != m_tour.next(m_swap_end))
        {
            m_starts.push_back(new_start);
            m_ends.push_back(m_swap_end);
            m_removes.push_back(new_remove);
            if (feasible())
            {
                // std::cout << "found feasible" << std::endl;
                check_best(total_remove - total_closing_add);
            }
            else if (m_save_nonsequential)
            {
                m_nonsequential_moves.push_back({m_starts, m_ends, m_removes});
                /*
                if (test_nonfeasible(root, tour, starts, ends, m_removes))
                {
                    return true;
                }
                */
            }
            m_starts.pop_back();
            m_ends.pop_back();
            m_removes.pop_back();
        }
    }
    //std::cout << m_starts.size() << std::endl;
    if (m_starts.size() >= m_kmax - 1)
    {
        return;
    }

    std::vector<primitives::point_id_t> points;
    const auto margin {total_remove - added};
    const auto search_box
    {
        m_tour.search_box(new_start, margin + m_tour.length(new_remove) + 1)
    };
    m_root.get_points(new_start, search_box, points);
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
        if (margin < add)
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
        m_starts.pop_back();
        m_ends.pop_back();
        m_removes.pop_back();
    }
}

// returns true if current swap does not break tour into multiple cycles.
// new edge i: (starts[i], ends[i])
// for p in m_removes: (p, next(p))
bool Finder::feasible() const
{
    // create and sort edges
    std::vector<BrokenEdge> edges;
    for (auto p : m_removes)
    {
        const auto sequence {m_tour.sequence(p, m_removes[0])};
        const auto first {p};
        const auto second {m_tour.next(p)};
        edges.push_back({first, second, sequence});
    }
    std::sort(std::begin(edges), std::end(edges)
        , [](const auto& lhs, const auto& rhs) { return lhs.sequence < rhs.sequence; });

    // maps to identify which removed edges points belong to and
    //  membership of new edges.
    std::unordered_map<primitives::point_id_t, size_t> edge_index;
    std::unordered_map<primitives::point_id_t, size_t> new_edges;
    for (size_t i {0}; i < edges.size(); ++i)
    {
        edge_index[edges[i].first] = i;
        edge_index[edges[i].second] = i;
        new_edges[m_starts[i]] = m_ends[i];
        new_edges[m_ends[i]] = m_starts[i];
    }

    // traversal
    const auto start {edges[0].first};
    auto current {start};
    size_t visited {0};
    size_t max_visited {m_starts.size() + m_ends.size()};
    do
    {
        if (edge_index.find(current) == std::cend(edge_index))
        {
            std::cout << "point not recognized" << std::endl;
            std::abort();
        }
        current = new_edges[current];
        ++visited;
        if (current == start)
        {
            break;
        }
        auto index {edge_index[current]};
        const auto edge {edges[index]};
        if (edge.first == current)
        {
            if (index == 0)
            {
                index = edges.size() - 1;
            }
            else
            {
                --index;
            }
            current = edges[index].second;
        }
        else
        {
            ++index;
            if (index == edges.size())
            {
                index = 0;
            }
            current = edges[index].first;
        }
        ++visited;
    } while (current != start and visited < max_visited);
    return current == start and visited == max_visited;
}
