#pragma once

#include "Config.h"
#include "Finder.h"
#include "MaskedFeasibleFinder.h"
#include "Tour.h"
#include "fileio.h"
#include "multicycle/multicycle.h"
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
template <typename FinderType = Finder>
bool initial_hill_climb(const Config& config, const point_quadtree::Node& root, Tour& tour)
{
    FinderType finder(root, tour);
    const auto kmax = config.get<size_t>("kmax", 4);
    const auto print_improvements = config.get<bool>("print_improvements", false);
    const auto suppress_output = config.get<bool>("suppress_output", false);
    const auto write_best = config.get<bool>("write_best", false);
    const auto save_period = config.get<size_t>("save_period", 1000);
    finder.set_kmax(kmax);
    int iteration {1};
    NanoTimer timer;
    timer.start();
    while (finder.find_best())
    {
        if (kmax == 2)
        {
            print_gmoves(timer, tour.size());
            print_work_ratio(finder.comparisons(), tour.size());
        }
        if (not suppress_output)
        {
            std::cout << "average points (kmax = " << kmax << "): " << finder.average_points() << std::endl;
        }

        const auto entry_ratio = tour.length_map()->entry_ratio();
        constexpr double max_entry_ratio {20};
        if (entry_ratio > max_entry_ratio and not suppress_output)
        {
            std::cout << "Clearing length map (max: "
                << max_entry_ratio << ")" << std::endl;
            tour.length_map()->clear();
        }
        if (print_improvements and not suppress_output)
        {
            std::cout << "improvement: " << finder.best_improvement()
                << " (length entry ratio: " << entry_ratio << ")" << std::endl;
        }
        tour.swap(finder.best_starts(), finder.best_ends(), finder.best_removes());
        ++iteration;
        if (write_best and (iteration % save_period) == 0 and not suppress_output)
        {
            std::cout << "current length: " << tour.length() << std::endl;
            fileio::write_ordered_points(tour.order(), "saves/test.tour");
        }

        timer.start();
    }
    if (iteration == 1 and not suppress_output)
    {
        if (kmax == 2)
        {
            print_gmoves(timer, tour.size());
            print_work_ratio(finder.comparisons(), tour.size());
        }
        std::cout << "average points (kmax = " << kmax << "): " << finder.average_points() << std::endl;
    }

    if (not suppress_output)
    {
        std::cout << __func__
            << ": tour length: " << tour.length()
            << " after " << iteration << " iterations."
            << std::endl;
    }
    return iteration > 1;
}

// true if improvement found.
template <typename FinderType = Finder>
bool hill_climb(const Config& config, const point_quadtree::Node& root, Tour& tour)
{
    FinderType finder(root, tour);

    const auto kmax = config.get<size_t>("kmax", 4);
    finder.set_kmax(kmax);
    int iteration {1};
    const auto suppress_output = config.get<bool>("suppress_output", false);
    while (finder.find_best())
    {
        if (not finder.nonsequential_improvement())
        {
            tour.swap(finder.best_starts(), finder.best_ends(), finder.best_removes());
        }
        if (not suppress_output)
        {
            std::cout << "iteration " << iteration
                << " current tour length: " << tour.length()
                << std::endl;
        }
        ++iteration;
    }
    std::cout << __func__
        << ": tour length: " << tour.length()
        << " after " << iteration << " iterations."
        << std::endl;
    std::cout << __func__
        << ": cycles: " << tour.cycles()
        << std::endl;
    const auto write_best = config.get<bool>("write_best", false);
    if (write_best)
    {
        fileio::write_ordered_points(tour.order(), "saves/test.tour");
    }
    return iteration > 1;
}

// true if improvement found.
bool try_nonsequential(const Config& config, const point_quadtree::Node& root, Tour& tour)
{
    const auto old_length = tour.length();
    Finder finder(root, tour);
    finder.save_nonsequential_moves();
    if (finder.find_best())
    {
        std::cout << "attempted nonsequential move when improvement exists!" << std::endl;
        return false;
    }
    std::cout << "Attempting "
        << finder.nonsequential_moves().size()
        << " nonsequential moves." << std::endl;
    std::map<int, int> k;
    int iteration {1};
    bool improved {false};
    for (const auto& move : finder.nonsequential_moves())
    {
        std::cout << "Attempting nonsequential move " << iteration
            << " (" << move.removes.size() << " edges replaced)"
            << std::endl;
        ++k[move.removes.size()];
        auto new_tour = tour;
        new_tour.multicycle_swap(move.starts, move.ends, move.removes);
        multicycle::merge(root, new_tour);
        hill_climb(config, root, new_tour);
        const auto new_length = new_tour.length();
        if (new_length < old_length)
        {
            std::cout << "found nonsequential improvement; new length: "
                << new_length
                << std::endl;
            tour = new_tour;
            improved = true;
            break;
        }
        ++iteration;
    }
    for (const auto& pair : k)
    {
        std::cout << pair.first << "-moves: " << pair.second << std::endl;
    }
    return improved;
}

// true if improvement found.
bool bottom_up_iteration(const point_quadtree::Node& root, Tour& tour, size_t kmax = 4, bool suppress_output = false)
{
    Finder finder(root, tour);
    finder.set_kmax(kmax);
    int iteration {1};
    size_t i {0};
    const auto length_set = finder.compute_length_set();
    std::cout << "min, max lengths: " << *std::cbegin(length_set)
        << ", " << *std::prev(std::cend(length_set)) << std::endl;
    for (auto length : length_set)
    {
        std::cout << i << " searching length " << i << ": " << length << std::endl;
        if (finder.find_best(length))
        {
            tour.swap(finder.best_starts(), finder.best_ends(), finder.best_removes());
            if (not suppress_output)
            {
                std::cout << "iteration " << iteration
                    << " current tour length: " << tour.length()
                    << std::endl;
            }
            return true;
        }
        ++i;
    }
    return false;
}

// true if improvement found.
bool segmented_hill_climb_iteration(const point_quadtree::Node& root, Tour& tour, size_t kmax = 4, size_t segment_size = 50, bool suppress_output = false)
{
    std::cout << "segmented hill climb" << std::endl;
    Finder finder(root, tour);
    finder.set_kmax(kmax);
    for (primitives::point_id_t i {0}; i < tour.size(); ++i)
    {
        if (finder.find_best(i, segment_size))
        {
            tour.swap(finder.best_starts(), finder.best_ends(), finder.best_removes());
            if (not suppress_output)
            {
                std::cout << "new tour length: " << tour.length() << std::endl;
            }
            return true;
        }
    }
    return false;
}

} // namespace hill_climb

