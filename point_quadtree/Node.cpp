#include "Node.h"

namespace point_quadtree {

Node::Node(const Box& box) : m_box(box) {}

void Node::insert(primitives::point_id_t i)
{
    m_points.push_back(i);
}

void Node::create_child(primitives::quadrant_t quadrant, const Box& box)
{
    if (m_children[quadrant])
    {
        return;
    }
    m_children[quadrant] = std::make_unique<Node>(box);
}

bool Node::touches(const Box& box) const
{
    return m_box.touches(box);
}

std::vector<primitives::point_id_t>
    Node::get_points(primitives::point_id_t i
    , const Box& search_box) const
{
    std::vector<primitives::point_id_t> points;
    get_points(i, search_box, points);
    return points;
}

void Node::get_points(primitives::point_id_t i
    , const Box& search_box
    , std::vector<primitives::point_id_t>& points) const
{
    if (m_points.empty())
    {
        for (const auto& unique_ptr : m_children)
        {
            if (unique_ptr and unique_ptr->touches(search_box))
            {
                unique_ptr->get_points(i, search_box, points);
            }
        }
    }
    else
    {
        for (const auto p : m_points)
        {
            points.push_back(p);
        }
    }
}

void Node::validate() const
{
    if (leaf())
    {
        if (empty())
        {
            throw std::logic_error("leaf node is empty!");
        }
    }
    else
    {
        if (not empty())
        {
            throw std::logic_error("non-leaf node has points!");
        }
        for (const auto& unique_ptr : m_children)
        {
            if (unique_ptr)
            {
                unique_ptr->validate();
            }
        }
    }
}

bool Node::leaf() const
{
    return std::all_of(std::cbegin(m_children)
        , std::cend(m_children)
        , [](const auto& child) { return not child; });
}

} // namespace point_quadtree
