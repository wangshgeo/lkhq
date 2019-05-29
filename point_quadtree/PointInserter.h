#pragma once

#include "constants.h"
#include "morton_keys.h"
#include "primitives.h"
#include "point_quadtree/Node.h"

#include <stdexcept>
#include <vector>

namespace point_quadtree {

class PointInserter
{
public:
    PointInserter(const std::vector<primitives::morton_key_t>& morton_keys
        , primitives::point_id_t point
        , point_quadtree::Node* current_node
        , primitives::depth_t current_depth);

    void place();

private:
    const std::vector<primitives::morton_key_t>& m_morton_keys;
    const primitives::point_id_t m_point {constants::invalid_point};
    const morton_keys::InsertionPath m_path;

    point_quadtree::Node*   m_current_node {nullptr};
    primitives::depth_t     m_current_depth {0}; // 0 is root.

};

} // namespace point_quadtree

