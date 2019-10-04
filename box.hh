#pragma once

#include "primitives.hh"

#include <iostream>
#include <algorithm>
#include <limits>

struct Box {
    primitives::space_t xmin {std::numeric_limits<primitives::space_t>::max()};
    primitives::space_t xmax {std::numeric_limits<primitives::space_t>::min()};
    primitives::space_t ymin {std::numeric_limits<primitives::space_t>::max()};
    primitives::space_t ymax {std::numeric_limits<primitives::space_t>::min()};

    bool touches(const Box& other) const {
        // TODO: tolerance?
        bool too_high   {ymin > other.ymax};
        bool too_low    {ymax < other.ymin};
        bool left       {xmax < other.xmin};
        bool right      {xmin > other.xmax};
        bool outside {too_high or too_low or left or right};
        return not outside;
    }

    // if x, y is not in this box, the box is enlarged to contain it.
    void include(double x, double y) {
        xmin = std::min(x, xmin);
        xmax = std::max(x, xmax);
        ymin = std::min(y, ymin);
        ymax = std::max(y, ymax);
    }
};

inline auto& operator<<(std::ostream& o, const Box& b) {
    o << "(" << b.xmin << ", " << b.ymin << ") "
        << "(" << b.xmax << ", " << b.ymax << ")" << std::endl;
    return o;
}
