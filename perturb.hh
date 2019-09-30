#pragma once

#include "hill_climb.hh"
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

}  // namespace perturb

