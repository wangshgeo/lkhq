#pragma once

// Children are indexed by Morton key quadrant.
// Only leaf nodes have points.

#include "Box.h"
#include <primitives.h>

#include <array>
#include <iostream>
#include <memory> // unique_ptr
#include <vector>

namespace point_quadtree {

class Node
{
public:
    Node(const Box&);

    void create_child(primitives::quadrant_t, const Box& box);
    const std::array<std::unique_ptr<Node>, 4>& children() const { return m_children; }
    Node* child(primitives::quadrant_t q) { return m_children[q].get(); }

    void insert(primitives::point_id_t i);

    const std::vector<primitives::point_id_t>& points() const { return m_points; }

    std::vector<primitives::point_id_t>
        get_points
        (primitives::point_id_t i, const Box& search_box) const;

    void increment_indirect() { ++m_indirect_points; }
    auto total_points() const { return m_indirect_points + m_points.size(); }

private:
    std::array<std::unique_ptr<Node>, 4> m_children;
    std::vector<primitives::point_id_t> m_points;
    size_t m_indirect_points {0}; // total number of points under the children of this node (not directly under this node).
    const Box m_box;

    bool touches(const Box&) const;
    void get_points(primitives::point_id_t i
        , const Box& search_box
        , std::vector<primitives::point_id_t>& points) const;
};

} // namespace point_quadtree
