#pragma once

#include "tour.hh"
#include "constants.h"

class MulticycleTour : public Tour
{
    using Tour::Adjacents;
public:
    MulticycleTour(const Tour& tour) : Tour(tour), cycle_id_(tour.size(), constants::INVALID_CYCLE) {}

    void multicycle_swap(const KMove &kmove);

    size_t min_cycle_size() const { return min_cycle_size_; }

    auto cycle_id(primitives::point_id_t i) const { return cycle_id_[i]; }
    bool split() const { return cycle_end_ > 1; }
    auto cycles() const { return cycle_end_; }

private:
    std::vector<primitives::cycle_id_t> cycle_id_;
    primitives::cycle_id_t cycle_end_ {1}; // one-past-the-last cycle id.
    size_t min_cycle_size_ {0};

    void update_multicycle();
    size_t update_next(const primitives::point_id_t start, const primitives::cycle_id_t cycle_id);

};

