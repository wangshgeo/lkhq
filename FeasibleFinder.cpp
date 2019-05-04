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
    // TODO: consider storing this neighborhood.
    std::vector<primitives::point_id_t> points;
    const auto search_box
    {
        m_tour.search_box(swap_start, remove + 1)
    };
    m_root.get_points(swap_start, search_box, points);
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
            if (feasible())
            {
                //m_tour.print();
                //print_move();
                check_best(improvement);
                return;
            }
            else // if (false) // TODO: non-sequential moves are a bit inefficient currently.
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
                    feasible();
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

    std::vector<primitives::point_id_t> points;
    const auto margin {total_remove - added};
    const auto search_box
    {
        m_tour.search_box(new_start, margin + m_tour.length(new_remove) + 1)
    };
    m_root.get_points(new_start, search_box, points);
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

// returns true if current swap does not break tour into multiple cycles.
// new edge i: (starts[i], ends[i])
// for p in m_removes: (p, next(p))
// TODO: misses certain subclass of feasible moves.
bool FeasibleFinder::feasible() const
{
    // create and sort edges
    std::vector<BrokenEdge> deleted_edges;
    for (auto p : m_removes)
    {
        const auto sequence {m_tour.sequence(p, m_removes[0])};
        const auto first {p};
        const auto second {m_tour.next(p)};
        deleted_edges.push_back({first, second, sequence});
    }
    std::sort(std::begin(deleted_edges), std::end(deleted_edges)
        , [](const auto& lhs, const auto& rhs) { return lhs.sequence < rhs.sequence; });
    /*
    auto print_deleted_edges = [&]()
    {
        std::cout << std::endl;
        for (const auto& e : deleted_edges)
        {
            std::cout << e << std::endl;
        }
        std::cout << std::endl;
    };
    print_deleted_edges();
    */

    // maps to identify which removed edges points belong to and
    //  membership of new edges.
    std::unordered_map<primitives::point_id_t, size_t> deleted_edge_index;
    std::unordered_map<primitives::point_id_t, std::vector<primitives::point_id_t>> new_edges;
    for (size_t i {0}; i < deleted_edges.size(); ++i)
    {
        deleted_edge_index[deleted_edges[i].first] = i;
        deleted_edge_index[deleted_edges[i].second] = i;
        new_edges[m_starts[i]].push_back(m_ends[i]);
        new_edges[m_ends[i]].push_back(m_starts[i]);
    }

    // traversal
    const auto start {deleted_edges[0].first};
    auto current {start};
    //std::cout << "New tour traversal:" << std::endl;
    //std::cout << current << std::endl;
    size_t visited {0};
    size_t max_visited {m_starts.size() + m_ends.size()};
    std::unordered_map<primitives::point_id_t, bool> visit_flag;
    visit_flag[current] = true;
    std::unordered_set<primitives::point_id_t> checklist;
    do
    {
        if (deleted_edge_index.find(current) == std::cend(deleted_edge_index))
        {
            std::cout << "point not recognized" << std::endl;
            std::abort();
        }
        // go to next in new edge.
        auto next = new_edges[current].back();
        if (new_edges[next].size() > 2)
        {
            std::cout << __func__ << ": error: too many adjacent points" << std::endl;
            std::abort();
        }
        if (visit_flag[next])
        {
            next = new_edges[current].front();
        }
        current = next;
        //std::cout << "new edge end: " << current << std::endl;
        visit_flag[current] = true;
        ++visited;
        if (current == start or checklist.find(current) != std::cend(checklist))
        {
            ++visited;
            break;
        }
        checklist.insert(current);
        // find adjacent new edge start point.
        auto index {deleted_edge_index[current]};
        const auto& edge {deleted_edges[index]};
        if (edge.first == current)
        {
            if (index == 0)
            {
                index = deleted_edges.size() - 1;
            }
            else
            {
                --index;
            }
            current = deleted_edges[index].second;
        }
        else
        {
            ++index;
            if (index == deleted_edges.size())
            {
                index = 0;
            }
            current = deleted_edges[index].first;
        }
        checklist.insert(current);
        //std::cout << "adjacent: " << current << std::endl;
        visit_flag[current] = true;
        ++visited;
    } while (current != start and visited < max_visited);
    //std::cout << std::endl;
    return current == start and visited == max_visited;
}

