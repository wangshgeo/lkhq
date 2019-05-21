#include "Mask.h"

Mask::Mask(size_t size) : m_active(size, 't') {}

void Mask::reset(const std::vector<primitives::point_id_t>& actives)
{
    std::fill(std::begin(m_active), std::end(m_active), 'f');
    for (auto i : actives)
    {
        m_active[i] = 't';
    }
}

