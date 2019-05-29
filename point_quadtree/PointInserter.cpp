#include "PointInserter.h"

namespace point_quadtree {

void PointInserter::place()
{
    //if (m_current_node->leaf())
    {
        if (m_current_node->empty())
        {
            throw std::logic_error("Found empty leaf node.");
        }
        m_current_node->insert(m_point);
        return;
    }

    //if (m_current_node->child())

}

PointInserter::PointInserter(const std::vector<primitives::morton_key_t>& morton_keys
    , primitives::point_id_t point
    , point_quadtree::Node* current_node
    , primitives::depth_t current_depth)
    : m_morton_keys(morton_keys)
    , m_point(point)
    , m_path(morton_keys::point_insertion_path(morton_keys[point]))
    , m_current_node(current_node)
    , m_current_depth(current_depth)
{
}

} // namespace point_quadtree

