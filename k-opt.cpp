#include "Config.h"
#include "Finder.h"
#include "NanoTimer.h"
#include "RandomFinder.h"
#include "OptimalFinder.h"
#include "Tour.h"
#include "fileio.h"
#include "hill_climb.h"
#include "length_stats.h"
#include "perturbers/SimpleDoubleBridge.h"
#include "point_quadtree/Domain.h"
#include "point_quadtree/point_quadtree.h"
#include "sweep.h"

#include <iostream>

int main(int argc, const char** argv)
{
    if (argc < 2)
    {
        std::cout << "Arguments: point_set_file_path optional_tour_file_path" << std::endl;
        return 0;
    }

    // Read input files.
    const auto [x, y] = fileio::read_coordinates(argv[1]);
    const auto initial_tour = fileio::initial_tour(argc, argv, x.size());

    // Config file (currently fixed).
    constexpr char config_path[] = "config.txt";
    std::cout << "Reading config file: " << config_path << std::endl;
    Config config(config_path);

    // Initial tour length calculation.
    point_quadtree::Domain domain(x, y);
    std::cout << "domain aspect ratio: " << domain.xdim(0) / domain.ydim(0) << std::endl;
    const auto print_domain_stats = config.get("domain_stats", false);
    if (print_domain_stats)
    {
        std::cout << "bounding x, y dim: "
            << domain.xdim(0) << ", " << domain.ydim(0)
            << std::endl;
    }
    Tour tour(&domain, initial_tour);
    std::cout << "Initial tour length: " << tour.length() << std::endl;

    // Quad tree.
    NanoTimer timer;
    timer.start();

    std::cout << "\nquadtree stats:\n";
    const auto root {point_quadtree::make_quadtree(x, y, domain)};
    std::cout << "node ratio: "
        << static_cast<double>(point_quadtree::count_nodes(root))
            / point_quadtree::count_points(root)
        << std::endl;
    std::cout << "Finished quadtree in " << timer.stop() / 1e9 << " seconds.\n\n";

    if (config.get("basic_hill_climb", true))
    {
        hill_climb::basic_hill_climb<OptimalFinder>(config, root, tour);
        std::cout << "hill climb final tour length: " << tour.length() << "\n\n";
    }
    if (config.get("random_finder", true))
    {
        while (true)
        {
            hill_climb::basic_hill_climb<RandomFinder>(config, root, tour);
            std::cout << "hill climb final tour length: " << tour.length() << "\n\n";
            // TODO: save tour if better
        }
    }
    if (config.get("experimental", false))
    {
        perturbers::SimpleDoubleBridge perturber;
        Finder finder(config, root, tour);
        while (sweep::perturb(config, finder, perturber));
    }

    const auto validate_tour = config.get("validate_tour", false);
    if (validate_tour)
    {
        tour.validate();
    }
    std::cout << "Final tour length: " << tour.length() << std::endl;
    const auto print_length_stats = config.get("length_stats", false);
    if (print_length_stats)
    {
        length_stats::print_lengths(tour);
    }
    return 0;
}
