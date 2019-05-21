#pragma once

// "Feasible" means single-cycle-preserving.
// There are feasible and non-feasible sequential moves.
// This finds improving, feasible, sequential moves.

#include "BoxMaker.h"
#include "Merger.h"
#include "Tour.h"
#include "cycle_check.h"
#include "point_quadtree/Node.h"
#include "primitives.h"

#include <algorithm> // fill
#include <unordered_map>
#include <unordered_set>
#include <vector>

class FeasibleFinder
{
public:
    FeasibleFinder(const point_quadtree::Node& root, Tour& tour)
        : m_root(root), m_tour(tour), m_box_maker(tour.x(), tour.y()) {}

    // returns true if an improving swap was found.
    bool find_best();

    const auto& best_starts() const { return m_best_starts; }
    const auto& best_ends() const { return m_best_ends; }
    const auto& best_removes() const { return m_best_removes; }
    primitives::length_t best_improvement() const { return m_best_improvement; }

    void set_kmax(size_t k) { m_kmax = k; }

    size_t comparisons() const { return m_comparisons; }

    double average_points() const { return static_cast<double>(m_point_sum) / m_point_neighborhoods; }

protected:
    const point_quadtree::Node& m_root;
    Tour& m_tour;
    size_t m_kmax {4};
    size_t m_comparisons {0};

    std::vector<primitives::point_id_t> m_starts; // start of new edge.
    std::vector<primitives::point_id_t> m_ends; // end of new edge.
    std::vector<primitives::point_id_t> m_removes; // start points of edges to remove.
    primitives::point_id_t m_swap_end {constants::invalid_point};

    std::vector<primitives::point_id_t> m_best_starts; // start of new edge.
    std::vector<primitives::point_id_t> m_best_ends; // end of new edge.
    std::vector<primitives::point_id_t> m_best_removes; // start points of edges to remove.
    primitives::length_t m_best_improvement {0};
    size_t m_point_sum {0};
    size_t m_point_neighborhoods {0};

    const BoxMaker m_box_maker;

    void start_search(const primitives::point_id_t swap_start
        , const primitives::point_id_t removed_edge);
    void search_both_sides(const primitives::length_t removed
        , const primitives::length_t added);
    void search_neighbors(const primitives::point_id_t new_start
        , const primitives::point_id_t new_remove
        , const primitives::length_t removed
        , const primitives::length_t added);

    void reset_search()
    {
        m_starts.clear();
        m_ends.clear();
        m_removes.clear();
        m_swap_end = constants::invalid_point;
        m_best_improvement = 0;
        m_comparisons = 0;
    }

    void check_best(primitives::length_t improvement)
    {
        if (improvement > m_best_improvement)
        {
            m_best_starts = m_starts;
            m_best_ends  = m_ends;
            m_best_removes = m_removes;
            m_best_improvement = improvement;
        }
    }

    bool gainful(primitives::length_t new_length, primitives::length_t removed_length) const
    {
        return new_length <= removed_length;
    }

    bool found_improvement() const
    {
        return m_best_improvement > 0;
    }

    template <typename T>
    void print_short_vec(const std::vector<T>& vec)
    {
        for(auto v : vec)
        {
            std::cout << "\t" << v;
        }
        std::cout << std::endl;
    }
    void print_move()
    {
        std::cout << "starts:";
        print_short_vec(m_starts);
        std::cout << "ends:";
        print_short_vec(m_ends);
        std::cout << "removes:";
        print_short_vec(m_removes);
    }
};

