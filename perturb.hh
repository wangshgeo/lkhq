#pragma once

#include "hill_climb.hh"
#include "hill_climber.hh"
#include "point_set.hh"
#include "tour.hh"
#include "primitives.hh"
#include "randomize/double_bridge.h"

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

}  // namespace perturb

