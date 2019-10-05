#pragma once

#include "hill_climb.hh"
#include "hill_climber.hh"
#include "kmove.hh"
#include "point_set.hh"
#include "primitives.hh"
#include "randomize/double_bridge.h"
#include "randomize/randomize.hh"
#include "tour.hh"

#include <optional>
#include <vector>

namespace perturb {

inline Tour perturb(const PointSet &point_set, const Tour &tour, size_t kmax) {
    auto new_tour = tour;
    randomize::double_bridge::swap(new_tour);
    hill_climb::hill_climb(point_set, new_tour, kmax);
    return new_tour;
}

inline Tour perturb(const HillClimber &hill_climber, const Tour &tour, size_t kmax) {
    auto new_hill_climber = hill_climber;
    auto new_tour = tour;
    const auto kmove = randomize::double_bridge::swap(new_tour);
    new_hill_climber.changed(kmove);
    hill_climb::hill_climb(new_hill_climber, new_tour, kmax);
    return new_tour;
}

// given a series of removed edges in tour sequence order, a k-swap connects the
// last point of a removed edge e to the first point of the removed edge e + 2.

// simple k-swap perturbation that reconnects every other edge (the densest possible kswap).
inline KMove dense_kswap(const std::vector<primitives::point_id_t> &ordered_points, size_t start, size_t kmax)
{
    KMove kmove;
    for (size_t k{0}; k < kmax; ++k) {
        const auto &s = (start + 2 * k) % ordered_points.size();
        kmove.removes.push_back(ordered_points[s]);
        kmove.starts.push_back(ordered_points[(s + 1) % ordered_points.size()]);
    }
    for (size_t k{0}; k < kmax; ++k) {
        kmove.ends.push_back(kmove.removes[(k + 2) % kmax]);
    }
    return kmove;
}

inline Tour dense_kswap(const HillClimber &hill_climber, const Tour &tour, size_t kmax, size_t swap_kmax) {
    auto new_hill_climber = hill_climber;
    auto new_tour = tour;
    const auto kmove = dense_kswap(new_tour.order(), randomize::sequence(new_tour.size()), swap_kmax);
    new_tour.swap(kmove);
    new_hill_climber.changed(kmove);
    hill_climb::hill_climb(new_hill_climber, new_tour, kmax);
    return new_tour;
}

}  // namespace perturb

