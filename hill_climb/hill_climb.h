#pragma once

#include "Config.h"
#include "tour.hh"
#include "fileio.h"
#include "point_quadtree/node.hh"
#include "primitives.hh"

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


/*
primitives::length_t hill_climb(const HillClimber &)
    const auto log_hillclimb = config.get<bool>("log_hillclimb", false);
    int iteration{0};
    const auto kmax = config.get<size_t>("kmax", 3);
    auto kmove = hill_climber.find_best(tour, kmax);
    while (kmove) {
        tour.swap(*kmove);
        if (log_hillclimb)
        {
            std::cout << "iteration " << iteration << " current tour length: " << tour.length() << std::endl;
        }
        kmove = hill_climber.find_best(tour, kmax);
        ++iteration;
    }
    const auto final_length = tour.length();
    if (log_hillclimb)
    {
        std::cout << "tour length after hill-climb: " << final_length
            << " (" << iteration << " iterations)" << std::endl;
    }
    write_if_better(final_length);
*/

} // namespace hill_climb

