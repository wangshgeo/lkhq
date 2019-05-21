#pragma once

// Base class for other search strategies.
// Makes sequential improvements.

#include "BoxMaker.h"
#include "Config.h"
#include "KMove.h"
#include "Tour.h"
#include "constants.h"
#include "cycle_check.h"
#include "point_quadtree/Node.h"
#include "primitives.h"

#include <optional>

class Finder
{
public:
    Finder(const Config& config, const point_quadtree::Node& root, Tour& tour)
        : m_root(root)
        , m_tour(tour)
        , m_box_maker(tour.x(), tour.y())
        , m_kmax(config.get<size_t>("kmax", m_kmax)) {}

    std::optional<KMove> find_best();

    template <typename PointContainer>
    std::optional<KMove> find_best(const std::optional<PointContainer>& starts);

    auto& tour() { return m_tour; }

private:
    const point_quadtree::Node& m_root;
    Tour& m_tour;
    const BoxMaker m_box_maker;
    size_t m_kmax {3};

    KMove m_kmove;
    primitives::point_id_t m_swap_end {constants::invalid_point};
    bool m_stop {false};

    void start_search(primitives::point_id_t i);
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

