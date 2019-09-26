#pragma once

#include "GenericFinder.h"

#include <algorithm> // min, max
#include <iterator> // next, prev
#include <limits>
#include <stdexcept> // logic_error
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace hill_climb {

class NonsequentialFinder : public GenericFinder<NonsequentialFinder>
{
    using base = GenericFinder<NonsequentialFinder>;
public:
    using base::base;
    void final_move_check();

    void find_best_nonsequential();

private:
    std::vector<KMove> m_nonsequential_moves;
    std::vector<KMove> m_double_cycle_moves;
    std::vector<KMove> m_single_cycle_moves;
    std::unordered_set<primitives::point_id_t> m_small_cycle;

    void try_combination(const KMove& kmove, size_t kmove_cycles, std::vector<KMove>::const_iterator begin);
    bool duplicate_remove(const KMove& kmove, const KMove& other) const;
    // returns 0 if cost greater than gain.
    primitives::length_t net_gain(const KMove& kmove) const;

    void print_double_cycle_stats() const;
    void try_double_cycle_merge(const KMove&);
};

void NonsequentialFinder::try_double_cycle_merge(const KMove&)
{
    //auto new_tour = m_tour;
    //new_tour.multicycle_swap();
}

void NonsequentialFinder::print_double_cycle_stats() const
{
    std::cout << "double-cycle moves found: " << m_double_cycle_moves.size() << std::endl;
    primitives::length_t min {std::numeric_limits<primitives::length_t>::max()};
    primitives::length_t max {0};
    double sum {0};
    for (const auto& kmove : m_double_cycle_moves)
    {
        auto net = net_gain(kmove);
        sum += net;
        min = std::min(min, net);
        max = std::max(max, net);
    }
    std::cout << "min, max, avg: "
        << min
        << ", " << max
        << ", " << sum / m_double_cycle_moves.size()
        << std::endl;
}

inline primitives::length_t NonsequentialFinder::net_gain(const KMove& kmove) const
{
    primitives::length_t cost {0};
    for (size_t i {0}; i < kmove.starts.size(); ++i)
    {
        cost += length(kmove.starts[i], kmove.ends[i]);
    }
    primitives::length_t gain {0};
    for (auto remove : kmove.removes)
    {
        gain += length(remove);
    }
    if (cost >= gain)
    {
        return 0;
    }
    return gain - cost;
}

inline void NonsequentialFinder::find_best_nonsequential()
{
    if (m_nonsequential_moves.size() < 2)
    {
        return;
    }
    const auto begin = std::cbegin(m_nonsequential_moves);
    auto kmove = begin;
    for (auto it = std::next(begin); it != std::cend(m_nonsequential_moves); ++it)
    {
        const auto kmove_cycles = cycle_check::count_cycles(m_tour, *kmove);
        if (kmove_cycles == 2)
        {
            m_double_cycle_moves.push_back(*kmove);
        }
        kmove = it;
    }
    print_double_cycle_stats();
    kmove = begin;
    for (auto it = std::next(begin); it != std::cend(m_nonsequential_moves); ++it)
    {
        const auto kmove_cycles = cycle_check::count_cycles(m_tour, *kmove);
        try_combination(*kmove, kmove_cycles, it);
        if (m_stop)
        {
            return;
        }
        kmove = it;
    }
    std::cout << "nonsequential moves seen: " << m_nonsequential_moves.size() << std::endl;
    std::cout << "single-cycle moves found: " << m_single_cycle_moves.size() << std::endl;
    print_double_cycle_stats();
}

void NonsequentialFinder::try_combination(const KMove& kmove
    , size_t kmove_cycles
    , std::vector<KMove>::const_iterator begin)
{
    double avg_cycles {0};
    size_t min_cycles {std::numeric_limits<size_t>::max()};
    size_t max_cycles {0};
    size_t tries {0};
    for (auto it = begin; it != std::cend(m_nonsequential_moves); ++it)
    {
        if (duplicate_remove(kmove, *it))
        {
            continue;
        }
        const auto new_kmove = kmove + *it;
        const auto new_kmove_cycles = cycle_check::count_cycles(m_tour, new_kmove);
        if (new_kmove_cycles <= kmove_cycles) // this threshold is quite arbitrary.
        {
            if (new_kmove_cycles == 1)
            {
                m_single_cycle_moves.push_back(new_kmove);
                m_stop = true;
                return;
            }
            if (new_kmove_cycles == 2)
            {
                m_double_cycle_moves.push_back(new_kmove);
            }
            try_combination(new_kmove, new_kmove_cycles, it);
            if (m_stop)
            {
                return;
            }
        }
        avg_cycles += new_kmove_cycles;
        min_cycles = std::min(min_cycles, new_kmove_cycles);
        max_cycles = std::max(max_cycles, new_kmove_cycles);
        ++tries;
    }
    if (false and tries > 0)
    {
        std::cout << "min, avg, max: "
            << min_cycles
            << ", " << avg_cycles / tries
            << ", " << max_cycles
            << std::endl;
    }
}

inline bool NonsequentialFinder::duplicate_remove(const KMove& kmove, const KMove& other) const
{
    std::unordered_set<primitives::point_id_t> seen;
    for (auto remove : kmove.removes)
    {
        seen.insert(remove);
    }
    for (auto remove : other.removes)
    {
        if (seen.find(remove) != std::cend(seen))
        {
            return true;
        }
    }
    return false;
}

inline void NonsequentialFinder::final_move_check()
{
    if (cycle_check::feasible(m_tour, m_kmove))
    {
        m_stop = true;
    }
    else
    {
        m_nonsequential_moves.push_back(m_kmove);
    }
}

}  // namespace hill_climb
