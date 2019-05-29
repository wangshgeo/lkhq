#pragma once

#include "GridPosition.h"
#include "Node.h"
#include "morton_keys.h"
#include <Box.h>
#include <primitives.h>

namespace point_quadtree {

Node make_quadtree(const std::vector<primitives::space_t>& x
    , const std::vector<primitives::space_t>& y
    , const Domain&);

size_t insert_points(point_quadtree::Node& root
    , const std::vector<primitives::morton_key_t>& morton_keys
    , const point_quadtree::Domain&);
size_t shallow_insert(const std::vector<primitives::morton_key_t>& morton_keys
    , primitives::point_id_t p, Node& root, const Domain& domain);

size_t initialize_points(point_quadtree::Node& root
    , const std::vector<primitives::morton_key_t>& morton_keys
    , const point_quadtree::Domain&);
size_t insert_point(const std::vector<primitives::morton_key_t>& morton_keys
    , primitives::point_id_t point_id, Node& root, const Domain&);

} // namespace point_quadtree


