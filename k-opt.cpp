#include "Finder.h"
#include "LengthMap.h"
#include "Tour.h"
#include "shuffler.h"
#include "fileio.h"
#include "multicycle/multicycle.h"
#include "point_quadtree/Domain.h"
#include "point_quadtree/point_quadtree.h"

#include <iostream>
#include <map>

// true if improvement found.
bool hill_climb(const point_quadtree::Node& root, Tour& tour, bool suppress_output = false)
{
    Finder finder(root, tour);
    int iteration {1};
    while (finder.find_best())
    {
        tour.swap(finder.best_starts(), finder.best_ends(), finder.best_removes());
        if (not suppress_output)
        {
            std::cout << "iteration " << iteration
                << " current tour length: " << tour.length()
                << std::endl;
        }
        ++iteration;
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
        hill_climb(root, new_tour, true);
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
            hill_climb(root, new_tour, true);
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
    const auto root {point_quadtree::make_quadtree(x, y, domain)};

    hill_climb(root, tour);
    shuffle(root, tour);

    /*
    bool improved {false};
    do
    {
        improved = false;
        improved |= hill_climb(root, tour);
        improved |= try_nonsequential(root, tour);
        std::cout << "Entries in length map: " << length_map.entries() << std::endl;
    } while (improved);
    if (constants::write_best)
    {
        fileio::write_ordered_points(tour.order(), "saves/test.tour");
    }
    std::cout << "final length: " << tour.length() << std::endl;
    */

    return 0;
}
