#pragma once

#include "hill_climb.hh"
#include "hill_climber.hh"
#include "kmove.hh"
#include "point_set.hh"
#include "primitives.hh"
#include "randomize/double_bridge.h"
#include "randomize/randomize.hh"
#include "tour.hh"
#include <point_quadtree/Domain.h>

#include <optional>
#include <vector>
#include <iterator>

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

// k-swap perturbation that selects completely random edges.
// kmax == 4: double-bridge.
inline KMove kswap(const std::vector<primitives::point_id_t> &ordered_points, size_t start, size_t kmax)
{
    const auto &n = ordered_points.size();
    const auto &ineligible_edges = kmax + 1;
    auto sequence_ids = randomize::random_set(0, n - ineligible_edges, kmax);
    primitives::sequence_t shift{0};
    std::transform(std::begin(sequence_ids), std::end(sequence_ids), std::begin(sequence_ids),
        [&start, &shift, &n](const auto &i) { return (start + i + shift++) % n; });
    KMove kmove;
    for (size_t k{0}; k < kmax; ++k) {
        const auto &s = sequence_ids[k];
        kmove.removes.push_back(ordered_points[s]);
        kmove.starts.push_back(ordered_points[(s + 1) % n]);
        const auto &s_end = sequence_ids[(k + 2) % kmax];
        kmove.ends.push_back(ordered_points[s_end]);
    }
    return kmove;
}
inline Tour kswap(const HillClimber &hill_climber, const Tour &tour, size_t kmax, size_t swap_kmax) {
    auto new_hill_climber = hill_climber;
    auto new_tour = tour;
    const auto kmove = kswap(new_tour.order(), randomize::sequence(new_tour.size()), swap_kmax);
    new_tour.swap(kmove);
    new_hill_climber.changed(kmove);
    hill_climb::hill_climb(new_hill_climber, new_tour, kmax);
    return new_tour;
}

inline Tour random_restart(const PointSet &point_set, const point_quadtree::Domain *domain, size_t kmax) {
    HillClimber hill_climber(point_set);
    const auto &n = domain->x().size();
    std::vector<primitives::point_id_t> random_order(n);
    for (primitives::point_id_t i{0}; i < n; ++i) {
        random_order[i] = i;
    }
    std::random_shuffle(std::begin(random_order), std::end(random_order));
    Tour tour(domain, random_order);
    hill_climb::hill_climb(hill_climber, tour, kmax);
    return tour;
}

inline KMove random_section(const std::vector<primitives::point_id_t> &ordered_points,
    primitives::sequence_t start,
    double random_fraction) {
    const auto &n = ordered_points.size();
    const primitives::sequence_t random_size = random_fraction * n;
    std::vector<primitives::point_id_t> random_order(random_size);
    for (primitives::sequence_t s{0}; s < random_size; ++s) {
        random_order[s] = s;
    }
    // first and last point in randomized sequence will not be shuffled,
    // but connected edges will still be deleted.
    std::random_shuffle(std::next(std::begin(random_order)), std::prev(std::end(random_order)));

    KMove kmove;
    for (auto it = std::cbegin(random_order); it != std::prev(std::cend(random_order)); ++it) {
        const auto &seq = (start + *it) % n;
        kmove.removes.push_back(ordered_points[seq]);
        kmove.starts.push_back(ordered_points[seq]);
        const auto &seq_next = (start + *std::next(it)) % n;
        kmove.ends.push_back(ordered_points[seq_next]);
    }
    return kmove;
}

inline Tour random_section(const HillClimber &hill_climber, const Tour &tour, size_t kmax, double random_fraction) {
    auto new_hill_climber = hill_climber;
    auto new_tour = tour;
    const auto kmove = random_section(new_tour.order(), randomize::sequence(new_tour.size()), random_fraction);
    new_tour.swap(kmove);
    new_hill_climber.changed(kmove);
    hill_climb::hill_climb(new_hill_climber, new_tour, kmax);
    return new_tour;
}

}  // namespace perturb

