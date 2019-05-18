#pragma once

#include "Tour.h"
#include "constants.h"
#include "point_quadtree/Node.h"
#include "primitives.h"

#include <vector>

struct Neighborhood
{
    Neighborhood() = default;
    Neighborhood(const point_quadtree::Node& root
        , const Tour& tour
        , primitives::point_id_t center_point
        , primitives::length_t radius)
        : center_point(center_point)
        , radius(radius)
        , near_points(tour.get_points(root, center_point, radius + 1))
    {
    }

    primitives::point_id_t center_point {constants::invalid_point};
    primitives::length_t radius {0};
    std::vector<primitives::point_id_t> near_points;
};

