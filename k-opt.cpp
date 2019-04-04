#include "Finder.h"
#include "LengthMap.h"
#include "Tour.h"
#include "fileio.h"
#include "point_quadtree/Domain.h"
#include "point_quadtree/point_quadtree.h"

#include <iostream>

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

    Finder finder(root, tour);

    while (finder.find_best())
    {
        //std::cout << "iteration" << std::endl;
        //std::cout << finder.starts().size() << std::endl;
        tour.swap(finder.best_starts(), finder.best_ends(), finder.best_removes());
        std::cout << tour.length() << std::endl;
    }
    if (constants::write_best)
    {
        fileio::write_ordered_points(tour.order(), "saves/test.tour");
    }

    return 0;
}
