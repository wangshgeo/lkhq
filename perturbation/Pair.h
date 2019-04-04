#pragma once

#include "Segment.h"

#include <algorithm> // min, max

struct Pair
{
    Pair(const Segment& a, const Segment& b) : min(std::min(a, b)), max(std::max(a, b)) {}
    Segment min;
    Segment max;

    bool operator==(const Pair& other) const
    {
        return min == other.min and max == other.max;
    }
    bool operator!=(const Pair& other) const
    {
        return not operator==(other);
    }
};
