#pragma once

#include "tour.hh"
#include "constants.h"

class MulticycleTour : public Tour
{
    using Adjacents = std::array<primitives::point_id_t, 2>;
public:
    MulticycleTour(const Tour& tour) : Tour(tour), m_cycle_id(tour.size(), constants::INVALID_CYCLE) {}

    void multicycle_swap(
        const std::vector<primitives::point_id_t>& starts
        , const std::vector<primitives::point_id_t>& ends
        , const std::vector<primitives::point_id_t>& removed_edges);
    void multicycle_swap(const KMove &kmove) { multicycle_swap(kmove.starts, kmove.ends, kmove.removes); }

    size_t min_cycle_size() const { return m_min_cycle_size; }

    auto cycle_id(primitives::point_id_t i) const { return m_cycle_id[i]; }
    bool split() const { return m_cycle_end > 1; }
    auto cycles() const { return m_cycle_end; }

private:
    std::vector<primitives::cycle_id_t> m_cycle_id;
    primitives::cycle_id_t m_cycle_end {1}; // one-past-the-last cycle id.
    size_t m_min_cycle_size {0};

    void update_multicycle();
    size_t update_next(const primitives::point_id_t start, const primitives::cycle_id_t cycle_id);

};

