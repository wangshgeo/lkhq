#include "Config.h"
#include "FeasibleFinder.h"
#include "LengthMap.h"
#include "NanoTimer.h"
#include "Tour.h"
#include "explorers.h"
#include "fileio.h"
#include "hill_climb.h"
#include "length_stats.h"
#include "perturbers/SimpleDoubleBridge.h"
#include "point_quadtree/Domain.h"
#include "point_quadtree/point_quadtree.h"
#include "sweep.h"

#include <iostream>
#include <map>
#include <stdexcept>

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
    const auto print_domain_stats = config.get<bool>("domain_stats", false);
    if (print_domain_stats)
    {
        std::cout << "bounding x, y dim: "
            << domain.xdim(0) << ", " << domain.ydim(0)
            << std::endl;
    }
    LengthMap length_map(x, y);
    Tour tour(&domain, initial_tour, &length_map);
    std::cout << "Initial tour length: " << tour.length() << std::endl;

    // Quad tree.
    NanoTimer timer;
    timer.start();
    const auto root {point_quadtree::make_quadtree(x, y, domain)};
    std::cout << root.box() << std::endl;
    for (const auto& ptr : root.children())
    {
        if (ptr)
        {
            std::cout << ptr->box() << std::endl;
        }
    }
    if (root.total_points() != tour.size())
    {
        throw std::logic_error("quadtree root did not count points accurately.");
    }
    std::cout << "Finished quadtree in " << timer.stop() / 1e9 << " seconds." << std::endl;

    if (config.get<bool>("basic_hill_climb", true))
    {
        hill_climb::basic_hill_climb<Finder>(config, root, tour);
        std::cout << "hill climb final tour length: " << tour.length() << std::endl;
    }

    if (config.get<bool>("experimental", false))
    {
        perturbers::SimpleDoubleBridge perturber;
        Finder finder(config, root, tour);
        while (sweep::perturb(config, finder, perturber));
        /*
        const auto average_outer_dim = 0.5 * (domain.xdim(0) + domain.ydim(0));
        hill_climb::neighborhood(config
            , root
            , 0.15 * average_outer_dim
            , tour);
        */
    }

    const auto validate_tour = config.get<bool>("validate_tour", false);
    if (validate_tour)
    {
        tour.validate();
    }
    std::cout << "Final tour length: " << tour.length() << std::endl;
    const auto print_length_stats = config.get<bool>("length_stats", false);
    if (print_length_stats)
    {
        length_stats::print_lengths(tour);
    }
    return 0;
}
