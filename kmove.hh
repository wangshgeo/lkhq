#pragma once

#include "primitives.hh"

#include <algorithm> // find, count
#include <stdexcept>
#include <vector>

struct KMove
{
    std::vector<primitives::point_id_t> starts;
    std::vector<primitives::point_id_t> ends;
    std::vector<primitives::point_id_t> removes; // removes edge i, next(i)

    auto current_k() const { return starts.size(); }

    bool removable(primitives::point_id_t i) const
    {
        return not contains(removes, i);
    }

    bool startable(primitives::point_id_t i) const
    {
        return std::count(std::cbegin(starts), std::cend(starts), i) < 2;
    }

    bool endable(primitives::point_id_t i) const
    {
        return std::count(std::cbegin(ends), std::cend(ends), i) < 2;
    }

    void clear()
    {
        starts.clear();
        ends.clear();
        removes.clear();
    }

    void validate() const
    {
        if (starts.size() != ends.size() or starts.size() != removes.size())
        {
            throw std::logic_error("invalid kmove.");
        }
    }

    KMove operator+(const KMove& other) const
    {
        auto new_kmove = *this;
        new_kmove.starts.insert(std::end(new_kmove.starts)
            , std::cbegin(other.starts)
            , std::cend(other.starts));
        new_kmove.ends.insert(std::end(new_kmove.ends)
            , std::cbegin(other.ends)
            , std::cend(other.ends));
        new_kmove.removes.insert(std::end(new_kmove.removes)
            , std::cbegin(other.removes)
            , std::cend(other.removes));
        return new_kmove;
    }

private:
    static bool contains(const std::vector<primitives::point_id_t>& points
        , primitives::point_id_t point)
    {
        return std::find(std::cbegin(points), std::cend(points), point) != std::cend(points);
    }

};


