#pragma once

#include "primitives.h"

#include <iostream>

struct Box
{
    primitives::space_t xmin {0};
    primitives::space_t xmax {0};
    primitives::space_t ymin {0};
    primitives::space_t ymax {0};

    bool touches(const Box& other) const
    {
        // TODO: tolerance?
        bool too_high   {ymin > other.ymax};
        bool too_low    {ymax < other.ymin};
        bool left       {xmax < other.xmin};
        bool right      {xmin > other.xmax};
        bool outside {too_high or too_low or left or right};
        return not outside;
    }
};

inline auto& operator<<(std::ostream& o, const Box& b)
{
    o << "(" << b.xmin << ", " << b.ymin << ") "
        << "(" << b.xmax << ", " << b.ymax << ")" << std::endl;
    return o;
}

