#pragma once

#include <constants.h>
#include <primitives.h>

#include <algorithm> // min, max

struct Segment
{
    Segment(primitives::point_id_t a, primitives::point_id_t b)
        : min(std::min(a, b)), max(std::max(a, b)) {}
    primitives::point_id_t min {constants::invalid_point};
    primitives::point_id_t max {constants::invalid_point};

    bool operator==(const Segment& other) const
    {
        return min == other.min and max == other.max;
    }
    bool operator!=(const Segment& other) const
    {
        return not operator==(other);
    }
    bool operator<(const Segment& other) const
    {
        return min < other.min;
    }
};

template <typename T>
T& operator<<(T& out, const Segment& s)
{
    return out << s.min << "," << s.max;
}
