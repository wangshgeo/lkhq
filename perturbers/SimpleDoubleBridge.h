#pragma once

#include <Tour.h>
#include <primitives.h>

#include <optional>
#include <vector>

namespace perturbers {

struct SimpleDoubleBridge
{
    //std::optional<std::vector<primitives::point_id_t>>
    auto operator()(primitives::point_id_t i, Tour& tour) const
    {
        constexpr size_t offset {1};
        constexpr size_t point_count {8 + 2 * offset};
        if (tour.size() < point_count)
        {
            throw std::logic_error("not enough points for double bridge.");
        }
        std::vector<primitives::point_id_t> points;
        while (points.size() < point_count)
        {
            points.push_back(i);
            i = tour.next(i);
        }

        KMove kmove;
        constexpr auto first_i {offset};
        constexpr auto first_j {first_i + 4};
        constexpr auto second_i {first_i + 2};
        constexpr auto second_j {second_i + 4};

        auto cycle_break = [&tour, &kmove](auto a, auto b)
        {
            kmove.removes.push_back(a);
            kmove.removes.push_back(b);

            kmove.starts.push_back(a);
            kmove.ends.push_back(tour.next(b));

            kmove.starts.push_back(tour.next(a));
            kmove.ends.push_back(b);
        };

        cycle_break(points[first_i], points[first_j]);
        cycle_break(points[second_i], points[second_j]);

        tour.swap(kmove);

        return std::nullopt;
    }
};

} // namespace perturbers

