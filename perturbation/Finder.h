#pragma once

#include "Swap.h"
#include <Tour.h>
#include <point_quadtree/Node.h>
#include <constants.h>
#include <primitives.h>
#include <forward.h>

#include <vector>

namespace perturbation {

class Finder
{
public:
    Finder(const point_quadtree::Node& root, Tour&
        , primitives::length_t target_cost = 0);

    bool search();
    void test_perturbations();

private:
    const point_quadtree::Node& m_root;
    Tour& m_tour;
    primitives::length_t m_target_cost {0};
    primitives::length_t m_next_cost {constants::max_length};

    std::vector<std::vector<primitives::point_id_t>> m_swaps;
    std::vector<bool> m_cyclic_firsts;
    primitives::length_t m_gain_relaxation {0};

    void find_forward_swap(
        const primitives::point_id_t global_swap_start
        , const primitives::point_id_t global_swap_end
        , const primitives::point_id_t edge_start
        , const primitives::length_t removed_length
        , const primitives::length_t added_length
        , std::vector<primitives::point_id_t>& swap
        , bool restrict_even_removals);

    void find_forward_swap();
    void find_forward_swap_ab();
};


} // namespace perturbation

