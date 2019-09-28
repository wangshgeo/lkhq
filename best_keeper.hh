#pragma once

#include "tour.hh"
#include "primitives.hh"
#include "constants.h"

class BestKeeper {
 public:
    BestKeeper(Tour &tour) : best_(tour), best_length_(tour.length()) {}

    template <typename HillClimberType, typename RandomizerType>
    void trial(HillClimberType &climber, RandomizerType &randomizer);

 private:
    Tour &best_;
    primitives::length_t best_length_{constants::MAX_COST};

};

template <typename HillClimberType, typename RandomizerType>
void BestKeeper::trial(HillClimberType &climber, RandomizerType &randomizer) {
    auto new_tour = best_;
}
