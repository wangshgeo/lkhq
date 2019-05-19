#pragma once

#include "point_quadtree/Node.h"
#include "primitives.h"

#include <vector>

namespace neighborhood {

using Points = std::vector<primitives::point_id_t>;

auto fixed_radius(const point_quadtree::Node& root, primitives::length_t /*radius*/)
{
    std::vector<Points> neighborhoods(root.total_points());
    return neighborhoods;
}

} // namespace neighborhood
