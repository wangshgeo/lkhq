#pragma once

#include "BoxMaker.h"
#include "FeasibleFinder.h"
#include "KMove.h"
#include "Mask.h"
#include "Tour.h"
#include "constants.h"
#include "point_quadtree/Node.h"
#include "primitives.h"

#include <optional>
#include <unordered_set>
#include <vector>

class MaskedFeasibleFinder
{
public:
    MaskedFeasibleFinder(const point_quadtree::Node& root, Tour& tour)
        : m_root(root)
        , m_tour(tour)
        , m_box_maker(tour.x(), tour.y())
        , m_mask(tour.size()) {}

    std::optional<KMove> find_best(const std::vector<primitives::point_id_t>& actives);
    std::optional<KMove> find_best();

private:
    const point_quadtree::Node& m_root;
    Tour& m_tour;
    const BoxMaker m_box_maker;

    Mask m_mask;
    const std::unordered_set<primitives::point_id_t>* m_active_set {nullptr};

    size_t m_kmax {5};

    KMove m_kmove;
    primitives::point_id_t m_swap_end {constants::invalid_point};
    bool m_stop {false};

    void start_search(const primitives::point_id_t swap_start
        , const primitives::point_id_t removed_edge);
    void delete_edge(const primitives::length_t removed
        , const primitives::length_t added);
    void add_edge(const primitives::point_id_t new_start
        , const primitives::point_id_t new_remove
        , const primitives::length_t removed
        , const primitives::length_t added);

    void reset_search();
    bool gainful(primitives::length_t new_length, primitives::length_t removed_length) const;

};

