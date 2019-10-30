#include "multicycle_tour.hh"

void MulticycleTour::multicycle_swap(const KMove &kmove) {
    apply_kmove(kmove);
    update_multicycle();
}

void MulticycleTour::update_multicycle() {
    std::fill(std::begin(cycle_id_), std::end(cycle_id_), constants::invalid_cycle);
    constexpr primitives::point_id_t FIRST_GROUP_START {0};
    primitives::point_id_t cycle_start {FIRST_GROUP_START};
    cycle_end_ = 0;
    min_cycle_size_ = std::numeric_limits<size_t>::max();
    while (cycle_start != constants::invalid_point)
    {
        auto cycle_size = update_next(cycle_start, cycle_end_);
        min_cycle_size_ = std::min(cycle_size, min_cycle_size_);
        //std::cout << "cycle size: " << cycle_size << std::endl;
        cycle_start = constants::invalid_point;
        for (primitives::point_id_t i {0}; i < size(); ++i)
        {
            if (cycle_id_[i] == constants::invalid_cycle)
            {
                cycle_start = i;
                break;
            }
        }
        ++cycle_end_;
    }
}

size_t MulticycleTour::update_next(const primitives::point_id_t start
    , const primitives::cycle_id_t cycle_id) {
    primitives::point_id_t current{start};
    next_[current] = adjacents_[current].front();
    primitives::point_id_t sequence {0};
    do {
        auto prev = current;
        sequence_[current] = sequence++;
        current = next_[current];
        next_[current] = get_other(current, prev);
        cycle_id_[current] = cycle_id;
        if (sequence > size()) {
            std::cout << __func__ << ": error: sequence is higher than total number of points." << std::endl;
            std::abort();
        }
    } while (current != start); // tour cycle condition.
    return sequence;
}

