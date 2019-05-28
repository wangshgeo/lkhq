#pragma once

// Base class for other search strategies.
// Makes sequential improvements.

#include "BoxMaker.h"
#include "Config.h"
#include "KMargin.h"
#include "KMove.h"
#include "Tour.h"
#include "constants.h"
#include "cycle_check.h"
#include "point_quadtree/Node.h"
#include "primitives.h"

#include <array>
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
    std::optional<KMove> find_best(std::nullopt_t) { return find_best(); }

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

    KMargin m_kmargin;

    void search(primitives::point_id_t i);
    void delete_both_edges();
    void try_nearby_points();

    void reset_search();

    primitives::length_t length(primitives::point_id_t a) const;
    primitives::length_t length(primitives::point_id_t a, primitives::point_id_t b) const;
};

template <typename PointContainer>
std::optional<KMove> Finder::find_best(const std::optional<PointContainer>& starts)
{
    for (auto i : starts)
    {
        search(i);
        if (m_stop)
        {
            return m_kmove;
        }
    }
    return std::nullopt;
}

