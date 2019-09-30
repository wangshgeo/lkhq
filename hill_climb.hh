#pragma once

#include "hill_climber.hh"
#include "point_set.hh"
#include "primitives.hh"
#include "tour.hh"

#include <iostream>

namespace hill_climb {

inline primitives::length_t hill_climb(const PointSet &point_set, Tour &tour, size_t kmax) {
    HillClimber hill_climber(point_set);
    auto kmove = hill_climber.find_best(tour, kmax);
    while (kmove) {
        tour.swap(*kmove);
        kmove = hill_climber.find_best(tour, kmax);
    }
    const auto length = tour.length();
    std::cout << "tour length: " << length << std::endl;
    return length;
}

} // namespace hill_climb

