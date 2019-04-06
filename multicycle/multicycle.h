#pragma once

#include "break_detection/break_detection.h"
#include "simple_merge/simple_merge.h"
#include <Tour.h>
#include <constants.h>
#include <point_quadtree/Node.h>
#include <primitives.h>

#include <algorithm> // find
#include <set>

namespace multicycle {

void merge(const point_quadtree::Node& root, Tour&);

std::array<std::vector<primitives::point_id_t>, 3> find_swap(Tour& tour
    , const point_quadtree::Node& root);

// Returns true if "swap" is improving.
bool search_neighbors(Tour& tour
    , const point_quadtree::Node& root
    , const primitives::point_id_t new_start
    , const primitives::point_id_t new_removal
    , const primitives::point_id_t global_swap_end
    , const primitives::length_t removed_length
    , const primitives::length_t added_length
    , std::vector<primitives::point_id_t>& starts
    , std::vector<primitives::point_id_t>& ends
    , std::vector<primitives::point_id_t>& removed_edges);

// Returns true if "swap" is improving.
bool search_both_sides(Tour& tour
    , const point_quadtree::Node& root
    , const primitives::point_id_t global_swap_end
    , const primitives::length_t removed_length
    , const primitives::length_t added_length
    , std::vector<primitives::point_id_t>& starts
    , std::vector<primitives::point_id_t>& ends
    , std::vector<primitives::point_id_t>& removed_edges);

std::array<std::vector<primitives::point_id_t>, 3> start_search(Tour& tour
    , const point_quadtree::Node& root
    , const primitives::point_id_t swap_start
    , const primitives::point_id_t removed_edge
    , const primitives::point_id_t swap_end);

bool test_nonfeasible(const point_quadtree::Node& root
    , Tour& tour
    , const std::vector<primitives::point_id_t>& starts
    , const std::vector<primitives::point_id_t>& ends
    , const std::vector<primitives::point_id_t>& removed_edges);

void optimize(const point_quadtree::Node& root, Tour&);

} // namespace multicycle
