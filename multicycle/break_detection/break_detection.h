#pragma once

#include "BrokenEdge.h"
#include <Tour.h>

#include <cstdlib>
#include <vector>
#include <unordered_map>

namespace multicycle {
namespace break_detection {

// returns true if swap does not break tour into multiple cycles.
// new edge i: (starts[i], ends[i])
// for p in removed_edges: (p, next(p))
inline bool feasible(
    const Tour& tour
    , const std::vector<primitives::point_id_t> starts
    , const std::vector<primitives::point_id_t> ends
    , const std::vector<primitives::point_id_t> removed_edges)
{
    // create and sort edges
    std::vector<BrokenEdge> edges;
    for (auto p : removed_edges)
    {
        const auto sequence {tour.sequence(p, removed_edges[0])};
        const auto first {p};
        const auto second {tour.next(p)};
        edges.push_back({first, second, sequence});
    }
    std::sort(std::begin(edges), std::end(edges)
        , [](const auto& lhs, const auto& rhs) { return lhs.sequence < rhs.sequence; });

    /*
    std::cout << "removed edges:" << std::endl;
    for (const auto& e : edges)
    {
        std::cout << e.first << ", " << e.second << " (" << e.sequence << ")" << std::endl;
    }
    */

    // maps to identify which removed edges points belong to and
    //  membership of new edges.
    std::unordered_map<primitives::point_id_t, size_t> edge_index;
    std::unordered_map<primitives::point_id_t, size_t> new_edges;
    for (size_t i {0}; i < edges.size(); ++i)
    {
        edge_index[edges[i].first] = i;
        edge_index[edges[i].second] = i;
        new_edges[starts[i]] = ends[i];
    }

    // traversal
    const auto start {edges[0].first};
    auto current {start};
    size_t visited {0};
    size_t max_visited {starts.size() + ends.size()};
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
            index = (index + 1) % edges.size();
            current = edges[index].first;
        }
        ++visited;
    } while (current != start and visited < max_visited);
    return current == start and visited == max_visited;
}

inline bool single_cycle(const Tour& tour)
{
    auto remaining {tour.size()};
    constexpr primitives::point_id_t start {0};
    auto current {start};
    do
    {
        current = tour.next(current);
        if (remaining == 0)
        {
            remaining = tour.size();
            break;
        }
        --remaining;
    }
    while (current != start);
    return current == start and remaining == 0;
}

} // namespace break_detection
} // namespace multicycle
