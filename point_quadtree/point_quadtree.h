#pragma once

#include "Domain.h"
#include "GridPosition.h"
#include "node.hh"
#include "PointInserter.h"
#include "morton_keys.h"
#include <Box.h>
#include <primitives.hh>

namespace point_quadtree {

Node make_quadtree(const std::vector<primitives::space_t>& x
    , const std::vector<primitives::space_t>& y
    , const Domain&);

void insert_points(const std::vector<primitives::space_t>& x
    , const std::vector<primitives::space_t>& y
    , const Domain& domain
    , Node& root);

size_t count_points(const Node& node);
size_t count_nodes(const Node& node);

void validate(const Node& node, primitives::depth_t depth = 0);

size_t max_leaf_points(const Node& node);
primitives::depth_t max_depth(const Node& node, primitives::depth_t depth = 0);

} // namespace point_quadtree


