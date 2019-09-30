#include "edge_map.hh"

#include <iostream>

namespace merge {

EdgeMap::EdgeMap(const EdgeSet &edge_set) {
    for (const auto &edge : edge_set) {
        insert(edge);
    }
}

auto EdgeMap::insert(const Edge &edge) -> std::optional<Points> {
    Points new_points;
    if (insert(edge.first, edge)) {
        new_points.push_back(edge.first);
    }
    if (insert(edge.second, edge)) {
        new_points.push_back(edge.second);
    }
    return new_points.empty() ? std::nullopt : make_optional(new_points);
}

auto EdgeMap::insert(const Edges &edges) -> std::optional<Points> {
    Points new_points;
    for (const auto &edge : edges) {
        const auto &points = insert(edge);
        if (points) {
            new_points.insert(std::cend(new_points), std::cbegin(*points), std::cend(*points));
        }
    }
    return new_points.empty() ? std::nullopt : make_optional(new_points);
}

bool EdgeMap::insert(primitives::point_id_t i, const Edge &edge) {
    if (map_.find(i) == std::cend(map_)) {
        map_[i].first = edge;
        return true;
    }
    // accept insertion of duplicates.
    if (*map_[i].first == edge or *map_[i].second == edge) {
        return false;
    }
    if (not map_[i].first) {
        throw std::logic_error("empty entry.");
    }
    if (map_[i].second) {
        throw std::logic_error("attempted to overfill an entry.");
    }
    map_[i].second = edge;
    return false;
}

auto EdgeMap::pop_edge() -> Edge {
    const auto it = std::begin(map_);
    auto edge = *it->second.first;
    remove_edges(edge);
    return edge;
}

void EdgeMap::remove_edges(const Edge &edge) {
    remove_edge(edge.first, edge);
    remove_edge(edge.second, edge);
}

void EdgeMap::remove_edge(primitives::point_id_t i, const Edge &edge) {
    auto it = map_.find(i);
    if (it == std::cend(map_)) {
        return;
    }
    if (*map_[i].first == edge) {
        std::swap(map_[i].first, map_[i].second);
        map_[i].second = std::nullopt;
        if (not map_[i].first) {
            map_.erase(it);
        }
    }
    if (*map_[i].second == edge) {
        map_[i].second = std::nullopt;
    }
}

auto EdgeMap::pop_edges(const Point &i) -> std::optional<Edges> {
    auto it = map_.find(i);
    if (it == std::cend(map_)) {
        return std::nullopt;
    }
    auto edge_pair = it->second;
    map_.erase(it);
    EdgeMap::Edges edges;
    if (edge_pair.first) {
        edges.push_back(*edge_pair.first);
    }
    if (edge_pair.second) {
        edges.push_back(*edge_pair.second);
    }
    return edges;
}

auto EdgeMap::pop_edges(const Points &points) -> std::optional<Edges> {
    Edges edges;
    for (const auto &i : points) {
        const auto popped = pop_edges(i);
        if (popped) {
            edges.insert(std::cend(edges), std::cbegin(*popped), std::cend(*popped));
        }
    }
    if (edges.empty()) {
        return std::nullopt;
    }
    return edges;
}

}  // namespace merge
