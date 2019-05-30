#include "point_quadtree.h"

namespace point_quadtree {

Node make_quadtree(const std::vector<primitives::space_t>& x
    , const std::vector<primitives::space_t>& y
    , const Domain& domain)
{
    GridPosition grid_position(domain);
    Node root(grid_position.make_box());
    insert_points(x, y, domain, root);
    root.validate();
    if (count_points(root) != x.size())
    {
        throw std::logic_error("quadtree root did not count points accurately.");
    }
    return root;
}

void insert_points(const std::vector<primitives::space_t>& x
    , const std::vector<primitives::space_t>& y
    , const Domain& domain
    , Node& root)
{
    const auto morton_keys
    {
        morton_keys::compute_point_morton_keys(x, y, domain)
    };
    GridPosition grid_position(domain);
    for (primitives::point_id_t i {0}; i < morton_keys.size(); ++i)
    {
        PointInserter inserter(grid_position
            , morton_keys
            , i
            , &root
            , 0);
    }
}

size_t count_points(const Node& node)
{
    if (node.leaf())
    {
        return node.size();
    }
    size_t counted {0};
    for (const auto& unique_ptr : node.children())
    {
        if (unique_ptr)
        {
            counted += count_points(*unique_ptr);
        }
    }
    return counted;
}

size_t count_nodes(const Node& node)
{
    size_t counted {1};
    for (const auto& unique_ptr : node.children())
    {
        if (unique_ptr)
        {
            counted += count_nodes(*unique_ptr);
        }
    }
    return counted;
}

} // namespace point_quadtree