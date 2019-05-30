#pragma once

#include "Domain.h"
#include "GridPosition.h"
#include "Node.h"
#include "PointInserter.h"
#include "morton_keys.h"
#include <Box.h>
#include <primitives.h>

namespace point_quadtree {

Node make_quadtree(const std::vector<primitives::space_t>& x
    , const std::vector<primitives::space_t>& y
    , const Domain&);

void insert_points(const std::vector<primitives::space_t>& x
    , const std::vector<primitives::space_t>& y
    , const Domain& domain
    , Node& root);
size_t shallow_insert(const std::vector<primitives::morton_key_t>& morton_keys
    , primitives::point_id_t p, Node& root, const Domain& domain);

size_t initialize_points(Node& root
    , const std::vector<primitives::morton_key_t>& morton_keys
    , const Domain&);
size_t insert_point(const std::vector<primitives::morton_key_t>& morton_keys
    , primitives::point_id_t point_id, Node& root, const Domain&);

size_t count_points(const Node& node);
size_t count_nodes(const Node& node);

} // namespace point_quadtree


