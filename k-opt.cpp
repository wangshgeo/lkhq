#include "Finder.h"
#include "LengthMap.h"
#include "Tour.h"
#include "fileio.h"
#include "point_quadtree/Domain.h"
#include "point_quadtree/point_quadtree.h"

#include <iostream>
#include <map>

void hill_climb(const point_quadtree::Node& root, Tour& tour)
{
    Finder finder(root, tour);
    int iteration {1};
    while (finder.find_best())
    {
        tour.swap(finder.best_starts(), finder.best_ends(), finder.best_removes());
        std::cout << "iteration " << iteration
            << " current tour length: " << tour.length()
            << std::endl;
        ++iteration;
    }
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

void try_nonsequential(const point_quadtree::Node& root, Tour& tour)
{
    // const auto old_length = tour.length();
    Finder finder(root, tour);
    finder.save_nonsequential_moves();
    if (finder.find_best())
    {
        std::cout << "attempted nonsequential move when improvement exists!" << std::endl;
        return;
    }
    std::cout << "Attempting "
        << finder.nonsequential_moves().size()
        << " nonsequential moves." << std::endl;
    std::map<int, int> k;
    for (const auto& move : finder.nonsequential_moves())
    {
        ++k[move.removes.size()];
    }
    for (const auto& pair : k)
    {
        std::cout << pair.first << "-moves: " << pair.second << std::endl;
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
    try_nonsequential(root, tour);
    if (constants::write_best)
    {
        fileio::write_ordered_points(tour.order(), "saves/test.tour");
    }
    std::cout << "Entries in length map: " << length_map.entries() << std::endl;

    return 0;
}
