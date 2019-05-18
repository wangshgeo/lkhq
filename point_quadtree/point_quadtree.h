#pragma once

#include "Node.h"
#include "morton_keys.h"
#include <Tour.h>
#include <primitives.h>

namespace point_quadtree {

inline primitives::grid_t quadrant_x(primitives::quadrant_t q)
{
    // assuming "N" curve; see morton_keys::interleave_coordinates for an explanation.
    switch(q)
    {
        case 0:
        case 1:
        {
            return 0;
        }
        case 2:
        case 3:
        {
            return 1;
        }
        default: return 0;
    }
}

inline primitives::grid_t quadrant_y(primitives::quadrant_t q)
{
    // assuming "N" curve; see morton_keys::interleave_coordinates for an explanation.
    switch(q)
    {
        case 0:
        case 2:
        {
            return 0;
        }
        case 1:
        case 3:
        {
            return 1;
        }
        default: return 0;
    }
}

inline void insert_point(
    const std::vector<primitives::morton_key_t>& morton_keys
    , primitives::point_id_t point_id
    , Node& root
    , const Domain& domain)
{
    auto point_destination {&root};
    primitives::depth_t depth {0};
    primitives::grid_t x {0};
    primitives::grid_t y {0};
    for (const auto quadrant : morton_keys::point_insertion_path(morton_keys[point_id]))
    {
        ++depth;
        x <<= 1;
        x += quadrant_x(quadrant);
        y <<= 1;
        y += quadrant_y(quadrant);
        auto child = point_destination->child(quadrant);
        if (not child)
        {
            Box box;
            box.xmin = x * domain.xdim(depth);
            box.ymin = y * domain.ydim(depth);
            box.xmax = (x + 1) * domain.xdim(depth);
            box.ymax = (y + 1) * domain.ydim(depth);
            point_destination->create_child(quadrant, box);
            child = point_destination->child(quadrant);
        }
        point_destination = child;
    }
    point_destination->insert(point_id);
}

inline void initialize_points(
    point_quadtree::Node& root
    , const std::vector<primitives::morton_key_t>& morton_keys
    , const point_quadtree::Domain& domain)
{
    for (primitives::point_id_t i {0}; i < morton_keys.size(); ++i)
    {
        point_quadtree::insert_point(morton_keys, i, root, domain);
    }
}

inline void print_search_pool_sizes(const Tour& tour, const Node& root)
{
    for (primitives::point_id_t i {0}; i < tour.size(); ++i)
    {
        const auto points_next = root.get_points(i, tour.search_box_next(i));
        std::cout << i << ": " << points_next.size();
        const auto points_prev = root.get_points(i, tour.search_box_prev(i));
        std::cout << ", " << points_prev.size();
        std::cout << std::endl;
    }
}

inline Node make_quadtree(
    const std::vector<primitives::space_t>& x
    , const std::vector<primitives::space_t>& y
    , const Domain& domain)
{

    const auto morton_keys
        {point_quadtree::morton_keys::compute_point_morton_keys(x, y, domain)};
    Box box;
    box.xmin = domain.xmin();
    box.ymin = domain.ymin();
    box.xmax = domain.xmin() + domain.xdim(0);
    box.ymax = domain.ymin() + domain.ydim(0);
    point_quadtree::Node root(box);
    point_quadtree::initialize_points(root, morton_keys, domain);
    return root;
}


} // namespace point_quadtree


