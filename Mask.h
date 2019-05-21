#pragma once

#include "primitives.h"

#include <vector>

class Mask
{
public:
    Mask(size_t size);

    void reset(const std::vector<primitives::point_id_t>& actives);
    bool active(primitives::point_id_t i) const { return m_active[i] == 't'; }
    void activate_all() { std::fill(std::begin(m_active), std::end(m_active), 't'); }

private:
    std::vector<char> m_active;

};

