#pragma once

// Local optima exploration strategies.

#include "Config.h"
#include "OldFinder.h"
#include "Tour.h"
#include "hill_climb.h"
#include "point_quadtree/Node.h"
#include "primitives.h"
#include "shuffler.h"

#include <map>

namespace explorers {

void double_bridge_explorer(
    const point_quadtree::Node& root
    , Tour& tour
    , const Config& config
    , size_t allowable_cost = 0)
{
    std::cout << __func__ << std::endl;
    std::map<primitives::length_t, size_t> frequency;
    auto best_length = tour.length();
    std::cout << "initial best tour length: " << best_length << std::endl;
    size_t max_local_optima {tour.size()};
    while(max_local_optima != 0)
    {
        auto test_tour = tour;
        test_tour.double_bridge_perturbation();
        hill_climb::initial_hill_climb<FeasibleFinder>(config, root, test_tour);
        auto length {test_tour.length()};
        if (length <= best_length + allowable_cost)
        {
            std::cout << "new best length: " << length
                << " (vs. " << best_length << ")" << std::endl;
            tour = test_tour;
            best_length = length;
        }
        --max_local_optima;
        std::cout << "local optimum length: " << length
            << " (remaining optima: " << max_local_optima << ")"
            << std::endl;
        ++frequency[length];
    }
    for (const auto& p : frequency)
    {
        std::cout << "length, frequency: " << p.first << ", " << p.second << std::endl;
    }
    std::cout << "best length found: " << std::begin(frequency)->first << std::endl;
}

void try_lateral(const Config& config, const point_quadtree::Node& root, Tour& tour)
{
    const auto old_length = tour.length();
    OldFinder finder(root, tour);
    finder.save_lateral_moves();
    if (finder.find_best())
    {
        std::cout << "attempted lateral move when improvement exists!" << std::endl;
        return;
    }
    std::cout << "Attempting "
        << finder.lateral_moves().size()
        << " lateral moves." << std::endl;
    int iteration {1};
    for (const auto& lateral : finder.lateral_moves())
    {
        std::cout << "Attempting lateral move " << iteration
            << " (" << lateral.removes.size() << " edges replaced)"
            << std::endl;
        auto new_tour = tour;
        new_tour.swap(lateral.starts, lateral.ends, lateral.removes);
        hill_climb::hill_climb(config, root, new_tour);
        if (new_tour.length() < old_length)
        {
            std::cout << "found lateral improvement" << std::endl;
            tour = new_tour;
            return;
        }
        ++iteration;
    }
}

void shuffle(const Config& config, const point_quadtree::Node& root, Tour& tour)
{
    auto old_length = tour.length();
    //const size_t shuffle_length {static_cast<size_t>(std::sqrt(tour.size()))};
    //const size_t shuffle_length {static_cast<size_t>(std::log(tour.size()))};
    //const size_t shuffle_length {30};
    const size_t shuffle_length {10};
    size_t iteration {1};
    while (true)
    {
        for (size_t i {0}; i < tour.size(); ++i)
        {
            const auto shuffled = shuffler::shuffle(tour.order(), i, shuffle_length);
            Tour new_tour(tour.domain(), shuffled, tour.length_map());
            hill_climb::hill_climb(config, root, new_tour);
            const auto new_length {new_tour.length()};
            std::cout << "iteration " << iteration
                << ", length " << new_length
                << " (best: " << old_length << ")"
                << std::endl;
            if (new_length < old_length)
            {
                old_length = new_length;
                tour = new_tour;
                std::cout << "new length: " << new_length << std::endl;
                if (constants::write_best)
                {
                    fileio::write_ordered_points(tour.order(), "saves/test.tour");
                }
                if (constants::quit_after_improvement)
                {
                    std::cout << "quitting after first improvement." << std::endl;
                    return;
                }
            }
            ++iteration;
        }
    }
}

} // namespace explorers
