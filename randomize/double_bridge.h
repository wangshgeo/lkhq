#pragma once

#include "primitives.hh"
#include "tour.hh"

#include <algorithm>
#include <array>
#include <random>

namespace randomize {
namespace double_bridge {

// random integer in [a, b].
inline primitives::point_id_t random_point(primitives::point_id_t a, primitives::point_id_t b) {
    static std::random_device device; // will be used to obtain a seed for the random number engine
    static std::mt19937 generator(device()); // standard mersenne_twister_engine seeded with random_device.
    std::uniform_int_distribution<primitives::point_id_t> distribution(a, b);
    return distribution(generator);
}

// selects at random 4 compatible points (sequence index) at which to perform the double bridge.
// {{edge_1_start, edge_2_start}, {edge_1_end, edge_2_end}}
std::array<primitives::point_id_t, 4> select_edges_to_remove(primitives::point_id_t n) {
    const auto start = random_point(0, n - 1);
    const auto first_end = random_point(4, n - 4);
    const auto second_start = random_point(2, first_end - 2);
    const auto second_end = random_point(first_end + 2, n - 2);
    std::array<primitives::point_id_t, 4> starts{0, second_start, first_end, second_end};
    std::transform(std::begin(starts), std::end(starts), std::begin(starts), [start, n](const auto &i){ return (start + i) % n; });
    return starts;
}

void swap(Tour& tour) {
    const auto removal = select_edges_to_remove(tour.size());
    std::array<primitives::point_id_t, 4> ends{
        removal[2] + 1,
        removal[3] + 1,
        removal[0] + 1,
        removal[1] + 1
    };
    tour.swap_sequence(removal, ends, removal);
}

}  // namespace double_bridge
}  // namespace randomize
