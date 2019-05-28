#pragma once

#include "primitives.h"

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


    // TODO: deprecate following public members.

    bool has_start(primitives::point_id_t start) const
    {
        return contains(starts, start);
    }

    bool has_end(primitives::point_id_t end) const
    {
        return contains(ends, end);
    }

    auto size() const { return starts.size(); }

    auto newest_point() const { return ends.back(); }

    void push_all(
        primitives::point_id_t start
        , primitives::point_id_t end
        , primitives::point_id_t remove)
    {
        push_deletion(start, remove);
        push_addition(end);
    }

    void pop_all()
    {
        pop_deletion();
        pop_addition();
    }

    void push_deletion(primitives::point_id_t new_start, primitives::point_id_t new_remove)
    {
        starts.push_back(new_start);
        removes.push_back(new_remove);
    }

    void push_addition(primitives::point_id_t new_end)
    {
        ends.push_back(new_end);
    }

    void pop_deletion()
    {
        starts.pop_back();
        removes.pop_back();
    }

    void pop_addition()
    {
        ends.pop_back();
    }

private:
    static bool contains(const std::vector<primitives::point_id_t>& points
        , primitives::point_id_t point)
    {
        return std::find(std::cbegin(points), std::cend(points), point) != std::cend(points);
    }

};


