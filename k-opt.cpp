#include "Finder.h"
#include "NanoTimer.h"
#include "LengthMap.h"
#include "CycleMaker.h"
#include "LateralFinder.h"
#include "FeasibleFinder.h"
#include "SimpleNonsequentialFinder.h"
#include "NonsequentialFinder.h"
#include "Tour.h"
#include "fileio.h"
#include "multicycle/multicycle.h"
#include "point_quadtree/Domain.h"
#include "point_quadtree/point_quadtree.h"
#include "shuffler.h"

#include <iostream>
#include <map>

constexpr size_t default_kmax {4};

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
bool initial_hill_climb(const point_quadtree::Node& root, Tour& tour, size_t kmax = 4)
{
    FinderType finder(root, tour);
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

        const auto entry_ratio = tour.length_map()->entry_ratio();
        constexpr double max_entry_ratio {20};
        if (entry_ratio > max_entry_ratio)
        {
            std::cout << "Clearing length map (max: "
                << max_entry_ratio << ")" << std::endl;
            tour.length_map()->clear();
        }
        if (constants::print_improvements)
        {
            std::cout << "improvement: " << finder.best_improvement()
                << " (length entry ratio: " << entry_ratio << ")" << std::endl;
        }
        tour.swap(finder.best_starts(), finder.best_ends(), finder.best_removes());
        ++iteration;
        if (constants::write_best and (iteration % constants::save_period) == 0)
        {
            std::cout << "current length: " << tour.length() << std::endl;
            fileio::write_ordered_points(tour.order(), "saves/test.tour");
        }

        timer.start();
    }
    if (iteration == 1 and kmax == 2)
    {
        print_gmoves(timer, tour.size());
        print_work_ratio(finder.comparisons(), tour.size());
    }

    std::cout << __func__
        << ": tour length: " << tour.length()
        << " after " << iteration << " iterations."
        << std::endl;
    return iteration > 1;
}

// true if improvement found.
template <typename FinderType = Finder>
bool hill_climb(const point_quadtree::Node& root, Tour& tour, size_t kmax = 4, bool suppress_output = false)
{
    FinderType finder(root, tour);
    finder.set_kmax(kmax);
    int iteration {1};
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
    if (constants::write_best)
    {
        fileio::write_ordered_points(tour.order(), "saves/test.tour");
    }
    return iteration > 1;
}

void try_lateral(const point_quadtree::Node& root, Tour& tour)
{
    const auto old_length = tour.length();
    Finder finder(root, tour);
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
        hill_climb(root, new_tour);
        if (new_tour.length() < old_length)
        {
            std::cout << "found lateral improvement" << std::endl;
            tour = new_tour;
            return;
        }
        ++iteration;
    }
}

// true if improvement found.
bool try_nonsequential(const point_quadtree::Node& root, Tour& tour)
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
        hill_climb(root, new_tour, default_kmax, true);
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

void shuffle(const point_quadtree::Node& root, Tour& tour)
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
            hill_climb(root, new_tour, default_kmax, true);
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

int main(int argc, const char** argv)
{
    if (argc < 2)
    {
        std::cout << "Arguments: point_set_file_path optional_tour_file_path" << std::endl;
        return 0;
    }

    // Read input files.
    const auto coordinates {fileio::read_coordinates(argv[1])};
    const auto& x {coordinates[0]};
    const auto& y {coordinates[1]};
    const auto initial_tour = fileio::initial_tour(argc, argv, x.size());

    // Distance calculation.
    point_quadtree::Domain domain(x, y);
    LengthMap length_map(x, y);
    Tour tour(&domain, initial_tour, &length_map);
    std::cout << "Initial tour length: " << tour.length() << std::endl;

    // Quad tree.
    NanoTimer timer;
    timer.start();
    const auto root {point_quadtree::make_quadtree(x, y, domain)};
    std::cout << "Finished quadtree in " << timer.stop() / 1e9 << " seconds." << std::endl;

    //hill_climb(root, tour, 5);
    for (size_t k {2}; k <= 5; ++k)
    {
        initial_hill_climb<FeasibleFinder>(root, tour, k);
    }
    /*
    for (size_t k {2}; k <= 7; ++k)
    {
        hill_climb<CycleMaker>(root, tour, k);
    }

    primitives::length_t current_cost {0};
    while(true)
    {
        std::cout << "current length: " << tour.length() << std::endl;
        SimpleNonsequentialFinder finder(root, tour);
        std::cout << "current cost: " << current_cost << std::endl;
        finder.find_best(current_cost);
        if (finder.nonsequential_improvement())
        {
            std::cout << "current length: " << tour.length() << std::endl;
            current_cost = 0;
        }
        else
        {
            current_cost = finder.next_cost();
            if (current_cost == std::numeric_limits<decltype(current_cost)>::max())
            {
                break;
            }
        }
    }
    */

    /*
    LateralFinder finder(root, tour);
    finder.set_kmax(5);
    finder.find_best();
    */

    //segmented_hill_climb_iteration(root, tour, 8, 30);
    //while(bottom_up_iteration(root, tour, 10));
    // shuffle(root, tour);

    /*
    bool improved {false};
    do
    {
        improved = false;
        improved |= hill_climb(root, tour);
        improved |= try_nonsequential(root, tour);
        std::cout << "Entries in length map: " << length_map.entries() << std::endl;
    } while (improved);
    */

    if (constants::write_best)
    {
        fileio::write_ordered_points(tour.order(), "saves/test.tour");
    }
    std::cout << "final length: " << tour.length() << std::endl;
    return 0;
}
