#include "cycle_check.h"

namespace cycle_check {

bool feasible(const Tour& tour
    , const std::vector<primitives::point_id_t>& starts
    , const std::vector<primitives::point_id_t>& ends
    , const std::vector<primitives::point_id_t>& removes)
{
    // create and sort edges
    std::vector<BrokenEdge> deleted_edges;
    for (auto p : removes)
    {
        const auto sequence {tour.sequence(p, removes[0])};
        const auto first {p};
        const auto second {tour.next(p)};
        deleted_edges.push_back({first, second, sequence});
    }
    std::sort(std::begin(deleted_edges), std::end(deleted_edges)
        , [](const auto& lhs, const auto& rhs) { return lhs.sequence < rhs.sequence; });

    // maps to identify which removed edges points belong to and
    //  membership of new edges.
    std::unordered_map<primitives::point_id_t, size_t> deleted_edge_index;
    std::unordered_map<primitives::point_id_t, std::vector<primitives::point_id_t>> new_edges;
    for (size_t i {0}; i < deleted_edges.size(); ++i)
    {
        deleted_edge_index[deleted_edges[i].first] = i;
        deleted_edge_index[deleted_edges[i].second] = i;
        new_edges[starts[i]].push_back(ends[i]);
        new_edges[ends[i]].push_back(starts[i]);
    }

    // traversal
    const auto start {deleted_edges[0].first};
    auto current {start};
    size_t visited {0};
    size_t max_visited {starts.size() + ends.size()};
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
        visit_flag[current] = true;
        ++visited;
    } while (current != start and visited < max_visited);
    return current == start and visited == max_visited;
}

} // namespace cycle_check
