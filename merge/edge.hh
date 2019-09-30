#pragma once

#include <primitives.hh>

#include <algorithm>
#include <utility>

namespace merge {

using Edge = std::pair<primitives::point_id_t, primitives::point_id_t>;

template <typename Ostream>
Ostream &operator<<(Ostream &out, const Edge &edge) {
    return out << edge.first << ", " << edge.second;
}

inline Edge make_edge(primitives::point_id_t i, primitives::point_id_t j) {
    return {std::min(i, j), std::max(i, j)};
}

}  // namespace merge

