#pragma once

#include "Config.h"
#include "Tour.h"
#include "fileio.h"
#include "point_quadtree/Node.h"
#include "primitives.h"

#include <iostream>
#include <map>

namespace hill_climb {

// only valid for 2-opt.
void print_gmoves(NanoTimer& timer, size_t n)
{
    auto dt = timer.stop() / 1e9;
    auto moves = (n - 1) * (n - 2) / 2 - 1; // 2-opt
    auto speed = moves / dt;
    std::cout << speed / 1e9 << " Gmoves / s" << std::endl;
}

// only valid for 2-opt.
void print_work_ratio(double comparisons, double n)
{
    auto moves = (n - 1) * (n - 2) / 2 - 1; // 2-opt
    std::cout << "work ratio: " << comparisons / moves << std::endl;
}

// true if improvement found.
template <typename FinderType>
bool basic_hill_climb(const Config& config, FinderType& finder)
{
    int iteration {1};
    const auto log_hillclimb = config.get<bool>("log_hillclimb", false);
    while (const auto kmove = finder.find_best())
    {
        finder.tour().swap(*kmove);
        if (log_hillclimb)
        {
            std::cout << "iteration " << iteration
                << " current tour length: " << finder.tour().length()
                << std::endl;
        }
        ++iteration;
    }
    if (log_hillclimb)
    {
        std::cout << __func__
            << ": tour length after hill-climb: " << finder.tour().length()
            << " (" << iteration << " iterations)"
            << std::endl;
    }
    return iteration > 1;
}

// true if improvement found.
template <typename FinderType>
bool basic_hill_climb(const Config& config, const point_quadtree::Node& root, Tour& tour)
{
    FinderType finder(config, root, tour);
    return basic_hill_climb(config, finder);
}

} // namespace hill_climb

