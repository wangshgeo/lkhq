#include "Config.h"
#include "NanoTimer.h"
#include "NonsequentialFinder.h"
#include "OptimalFinder.h"
#include "RandomFinder.h"
#include "Tour.h"
#include "fileio.h"
#include "hill_climb.h"
#include "length_stats.h"
#include "point_quadtree/Domain.h"
#include "point_quadtree/point_quadtree.h"

#include <filesystem>
#include <iostream>

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
            throw std::runtime_error("output_filename parent directory does not exist.");
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

    if (config.get("basic_hill_climb", false))
    {
        hill_climb::basic_hill_climb<OptimalFinder>(config, root, tour);
        const auto new_length = tour.length();
        std::cout << "hill climb final tour length: " << new_length << "\n\n";
        write_if_better(new_length);
    }
    if (config.get("nonsequential", false))
    {
        NonsequentialFinder finder(config, root, tour);
        finder.find_best();
        finder.find_best_nonsequential();
        const auto new_length = tour.length();
        std::cout << "nonsequential final tour length: " << new_length << "\n\n";
        write_if_better(new_length);
    }
    if (config.get("random_finder", false))
    {
        while (true)
        {
            hill_climb::basic_hill_climb<RandomFinder>(config, root, tour);
            const auto new_length = tour.length();
            std::cout << "hill climb final tour length: " << new_length << "\n\n";
            write_if_better(new_length);
        }
    }
    if (config.get("experimental", false))
    {
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
