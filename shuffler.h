#pragma once

#include "primitives.h"

#include <algorithm>    // rotate, random_shuffle
#include <cstdint>      // size_t
#include <vector>

namespace shuffler {

inline std::vector<primitives::point_id_t> shuffle(const std::vector<primitives::point_id_t>& order
    , size_t start, size_t length)
{
    auto new_order = order;
    const auto begin = std::begin(new_order) + start;
    if (start + length > new_order.size())
    {
        std::rotate(std::begin(new_order), begin, std::end(new_order));
        std::random_shuffle(std::begin(new_order), std::begin(new_order) + length);
    }
    else
    {
        std::random_shuffle(begin, begin + length);
    }
    return new_order;
}

} // namespace shuffler

