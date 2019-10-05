#pragma once

#include <random>
#include <primitives.hh>

namespace randomize {

// random integer in [a, b].
inline primitives::point_id_t random_point(primitives::sequence_t a, primitives::sequence_t b) {
    static std::random_device device; // will be used to obtain a seed for the random number engine
    static std::mt19937 generator(device()); // standard mersenne_twister_engine seeded with random_device.
    std::uniform_int_distribution<primitives::point_id_t> distribution(a, b);
    return distribution(generator);
}

inline primitives::sequence_t sequence(primitives::sequence_t max) {
    return random_point(0, max);
}

}  // namespace randomize


