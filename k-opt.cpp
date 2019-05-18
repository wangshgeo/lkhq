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

    // Config file (currently fixed).
    constexpr char config_path[] = "config.txt";
    std::cout << "Reading config file: " << config_path << std::endl;
    Config config(config_path);

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

    hill_climb::initial_hill_climb<FeasibleFinder>(config, root, tour);

    return 0;
}
