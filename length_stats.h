#pragma once

#include "length_calculator.hh"
#include "tour.hh"
#include "primitives.hh"

#include <iostream>
#include <map>

namespace length_stats {

void print_lengths(const Tour& tour)
{
    const LengthCalculator length_calculator(tour.x(), tour.y());
    std::map<primitives::length_t, size_t> lengths;
    for (primitives::point_id_t i {0}; i < tour.size(); ++i)
    {
        auto length = length_calculator(i, tour.next(i));
        ++lengths[length];
    }
    for (const auto& pair : lengths)
    {
        std::cout << "length, frequency: "
            << pair.first << "\t" << pair.second
            << std::endl;
    }
}

} // namespace length_stats


