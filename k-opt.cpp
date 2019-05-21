#include "Config.h"
#include "FeasibleFinder.h"
#include "LengthMap.h"
#include "NanoTimer.h"
#include "Tour.h"
#include "explorers.h"
#include "fileio.h"
#include "hill_climb.h"
#include "point_quadtree/Domain.h"
#include "point_quadtree/point_quadtree.h"

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
    LengthMap length_map(x, y);
    Tour tour(&domain, initial_tour, &length_map);
    std::cout << "Initial tour length: " << tour.length() << std::endl;

    // Quad tree.
    NanoTimer timer;
    timer.start();
    const auto root {point_quadtree::make_quadtree(x, y, domain)};
    if (root.total_points() != tour.size())
    {
        throw std::logic_error("quadtree root did not count points accurately.");
    }
    std::cout << "Finished quadtree in " << timer.stop() / 1e9 << " seconds." << std::endl;

    hill_climb::initial_hill_climb<FeasibleFinder>(config, root, tour);

    /*
    const auto average_outer_dim = 0.5 * (domain.xdim(0) + domain.ydim(0));
    hill_climb::neighborhood(config
        , root
        , 0.15 * average_outer_dim
        , tour);
    */

    std::cout << "Final tour length: " << tour.length() << std::endl;
    return 0;
}
