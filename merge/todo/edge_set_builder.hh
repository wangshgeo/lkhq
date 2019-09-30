#pragma once

#include "edge.hh"
#include "edge_set.hh"

#include <primitives.hh>

#include <set>

namespace merge {

class EdgeSetBuilder {
    using Point = primitives::point_id_t;
    using Points = std::vector<Point>;
 public:
    // returns new points.
    Points insert(const Edge &edge) {
       Points new_points;
       const auto check_new = [this, &new_points](const auto &i) {
           if (points_.find(edge.first) == std::cend(points_)) {
               new_points.push_back(edge.first);
               points_.insert(edge.first);
           }
       };
       check_new(edge.first);
       check_new(edge.second);
       edges_.insert(edge);
       return new_points;
    }

 private:
    EdgeSet edges_;
    using PointSet = std::set<Point>;
    PointSet points_;
};

}  // namespace merge

