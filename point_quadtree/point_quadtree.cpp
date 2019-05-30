#include "point_quadtree.h"

namespace point_quadtree {

Node make_quadtree(const std::vector<primitives::space_t>& x
    , const std::vector<primitives::space_t>& y
    , const Domain& domain)
{
    const auto morton_keys
    {
        morton_keys::compute_point_morton_keys(x, y, domain)
    };
    GridPosition grid_position(domain);
    Node root(grid_position.make_box());
    const auto nodes_created = initialize_points(root, morton_keys, domain);
    std::cout << "quadtree node ratio: " << double(nodes_created) / x.size() << std::endl;
    root.validate();
    if (root.total_points() != x.size())
    {
        throw std::logic_error("quadtree root did not count points accurately.");
    }
    return root;
}

void insert_points(const std::vector<primitives::space_t>& x
    , const std::vector<primitives::space_t>& y
    , const Domain& domain
    , Node&)
{
    const auto morton_keys
    {
        morton_keys::compute_point_morton_keys(x, y, domain)
    };
    for (primitives::point_id_t i {0}; i < morton_keys.size(); ++i)
    {
        //shallow_insert(morton_keys, i, root, domain);
    }
}

size_t shallow_insert(const std::vector<morton_keys::InsertionPath>& insertion_paths
    , primitives::point_id_t p
    , Node& root
    , const Domain& domain)
{
    if (root.empty())
    {
        root.insert(p);
        return 0;
    }
    auto point_destination {&root};
    GridPosition grid_position(domain);
    size_t nodes_created {0};
    for (const auto quadrant : insertion_paths[p]) // morton_keys::point_insertion_path(morton_keys[p]))
    {
        if (point_destination->empty())
        {
        }
        point_destination->increment_indirect();
        grid_position.descend(quadrant);
        auto child = point_destination->child(quadrant);
        if (not child)
        {
            point_destination->create_child(quadrant, grid_position.make_box());
            ++nodes_created;
            child = point_destination->child(quadrant);
        }
        point_destination = child;
    }
    point_destination->insert(p);
    return nodes_created;
}

size_t initialize_points(Node& root
    , const std::vector<primitives::morton_key_t>& morton_keys
    , const Domain& domain)
{
    size_t total_nodes_created {0};
    for (primitives::point_id_t i {0}; i < morton_keys.size(); ++i)
    {
        total_nodes_created += insert_point(morton_keys, i, root, domain);
    }
    return total_nodes_created;
}

size_t insert_point(const std::vector<primitives::morton_key_t>& morton_keys
    , primitives::point_id_t point_id
    , Node& root
    , const Domain& domain)
{
    auto point_destination {&root};
    GridPosition grid_position(domain);
    size_t nodes_created {0};
    for (const auto quadrant : morton_keys::point_insertion_path(morton_keys[point_id]))
    {
        point_destination->increment_indirect();
        grid_position.descend(quadrant);
        auto child = point_destination->child(quadrant);
        if (not child)
        {
            point_destination->create_child(quadrant, grid_position.make_box());
            ++nodes_created;
            child = point_destination->child(quadrant);
        }
        point_destination = child;
    }
    point_destination->insert(point_id);
    return nodes_created;
}

} // namespace point_quadtree
