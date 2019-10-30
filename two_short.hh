#pragma once

#include "tour.hh"
#include "point_set.hh"
#include "edge.hh"
#include "primitives.hh"
#include "kmove.hh"
#include "randomize/randomize.hh"

#include <array>
#include <random>
#include <vector>
#include <set>

namespace two_short {

std::set<edge::Edge> get_short_edges(const PointSet &point_set, const Tour &tour);
KMove make_perturbation(const Tour &tour, std::vector<edge::Edge> &short_edges);

}  // namespace two_short


