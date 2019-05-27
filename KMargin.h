#pragma once

#include "primitives.h"

#include <vector>

struct KMargin
{
    std::vector<primitives::length_t> increments;
    primitives::length_t total {0};

    void add_increment(primitives::length_t increment)
    {
        total += increment;
        increments.push_back(increment);
    }

    void remove_increment()
    {
        total -= increments.back();
        increments.pop_back();
    }

    void clear()
    {
        increments.clear();
        total = 0;
    }
};



