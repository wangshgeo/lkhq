#include "Config.h"
#include "NanoTimer.h"
#include "hill_climb/NonsequentialFinder.h"
#include "hill_climber.hh"
#include "hill_climb.hh"
#include "hill_climb/RandomFinder.h"
#include "tour.hh"
#include "perturb.hh"
#include "check.hh"
#include "merge/merge.hh"
#include "randomize/double_bridge.h"
#include "fileio.h"
#include "length_stats.h"
#include "point_quadtree/Domain.h"
#include "point_quadtree/point_quadtree.h"

#include <filesystem>
#include <iostream>
#include <fstream>

int main(int argc, const char** argv)
{
    if (argc < 2)
    {
        std::cout << "Arguments: point_set_file_path optional_tour_file_path" << std::endl;
        return 0;
    }

    // Config file (currently fixed).
    constexpr char config_path[] = "config.txt";
    std::cout << "Reading config file: " << config_path << std::endl;
    Config config(config_path);
    if (config.has("output_filename"))
    {
        const std::filesystem::path output_path(config.get("output_filename"));
        const auto parent_path = output_path.parent_path();
        if (not std::filesystem::exists(parent_path))
        {
            std::filesystem::create_directory(parent_path);
        }
    }

    // Read input files.
    const auto [x, y] = fileio::read_coordinates(argv[1]);
    const auto initial_tour = fileio::initial_tour(argc, argv, x.size());

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
    const auto initial_tour_length = tour.length();
    std::cout << "Initial tour length: " << initial_tour_length << std::endl;

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

    auto best_length = initial_tour_length;
    auto write_if_better = [&config, &tour, &best_length](primitives::length_t new_length)
    {
        if (new_length < best_length)
        {
            if (config.get("write_best", false))
            {
                fileio::write_ordered_points(tour.order(), config.get("output_filename"));
            }
        }
    };

    PointSet point_set(root, x, y);
    if (config.get("basic_hill_climb", false))
    {
        const auto kmax = config.get<size_t>("kmax", 3);
        auto new_length = hill_climb::hill_climb(point_set, tour, kmax);
        if (new_length < best_length) {
            best_length = new_length;
            std::cout << "improvement: " << new_length << std::endl;
        }
        write_if_better(new_length);

        const auto new_tour = perturb::perturb(point_set, tour, kmax);
        check::check_tour(new_tour);
        write_if_better(new_tour.length());

        merge::merge(tour, new_tour);
    }

    return EXIT_SUCCESS;
}
