#pragma once

#include "primitives.h"

#include <algorithm> // find
#include <vector>

struct KMove
{
    std::vector<primitives::point_id_t> starts;
    std::vector<primitives::point_id_t> ends;
    std::vector<primitives::point_id_t> removes; // removes edge i, next(i)

    auto size() const { return starts.size(); }

    auto newest_point() const { return ends.back(); }

    bool removable(primitives::point_id_t i) const
    {
        return not contains(removes, i);
    }

    bool has_start(primitives::point_id_t start) const
    {
        return contains(starts, start);
    }

    bool has_end(primitives::point_id_t end) const
    {
        return contains(ends, end);
    }

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

    void clear()
    {
        starts.clear();
        ends.clear();
        removes.clear();
    }

private:
    static bool contains(const std::vector<primitives::point_id_t>& points
        , primitives::point_id_t point)
    {
        return std::find(std::cbegin(points), std::cend(points), point) != std::cend(points);
    }

};


