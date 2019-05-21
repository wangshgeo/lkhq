#pragma once

// Deprecated; capable of sequential and nonsequential moves.
// Also saves lateral (non-improving) moves for perturbations.

#include "BrokenEdge.h"
#include "Merger.h"
#include "Tour.h"
#include "point_quadtree/Node.h"
#include "primitives.h"

#include <algorithm> // fill
#include <vector>
#include <set>

class OldFinder
{
public:
    OldFinder(const point_quadtree::Node& root, Tour& tour)
        : m_root(root), m_tour(tour), m_searchable(tour.size(), true) {}

    // returns true if an improving swap was found.
    bool find_best();
    bool find_best(primitives::length_t);
    bool find_best(primitives::point_id_t start, size_t size);

    const std::vector<primitives::point_id_t>& best_starts() const { return m_best_starts; }
    const std::vector<primitives::point_id_t>& best_ends() const { return m_best_ends; }
    const std::vector<primitives::point_id_t>& best_removes() const { return m_best_removes; }

    struct Move
    {
        std::vector<primitives::point_id_t> starts;
        std::vector<primitives::point_id_t> ends;
        std::vector<primitives::point_id_t> removes;
    };
    const std::vector<Move>& lateral_moves() const { return m_lateral_moves; }
    void save_lateral_moves() { m_save_lateral_moves = true; }
    const std::vector<Move>& nonsequential_moves() const { return m_nonsequential_moves; }
    void save_nonsequential_moves() { m_save_nonsequential = true; }

    std::set<primitives::length_t> compute_length_set();

    void set_kmax(size_t k) { m_kmax = k; }

    bool nonsequential_improvement() const { return m_nonsequential_improvement; }

private:
    const point_quadtree::Node& m_root;
    Tour& m_tour;
    size_t m_kmax {4};
    bool m_first_improvement {true};
    bool m_save_lateral_moves {false};
    bool m_save_nonsequential {false};
    bool m_restricted {false};
    bool m_nonsequential_improvement {false};
    std::vector<bool> m_searchable;

    std::vector<Move> m_lateral_moves;
    std::vector<Move> m_nonsequential_moves;

    std::vector<primitives::point_id_t> m_starts; // start of new edge.
    std::vector<primitives::point_id_t> m_ends; // end of new edge.
    std::vector<primitives::point_id_t> m_removes; // start points of edges to remove.
    primitives::point_id_t m_swap_end {constants::invalid_point};

    std::vector<primitives::point_id_t> m_best_starts; // start of new edge.
    std::vector<primitives::point_id_t> m_best_ends; // end of new edge.
    std::vector<primitives::point_id_t> m_best_removes; // start points of edges to remove.
    primitives::length_t m_best_improvement {0};

    void start_search(const primitives::point_id_t swap_start
        , const primitives::point_id_t removed_edge);
    void search_both_sides(const primitives::length_t removed
        , const primitives::length_t added);
    void search_neighbors(const primitives::point_id_t new_start
        , const primitives::point_id_t new_remove
        , const primitives::length_t removed
        , const primitives::length_t added);

    bool feasible() const;

    void reset_search()
    {
        m_starts.clear();
        m_ends.clear();
        m_removes.clear();
        m_swap_end = constants::invalid_point;
        m_best_improvement = 0;
        m_lateral_moves.clear();
        m_nonsequential_moves.clear();
        m_nonsequential_improvement = false;
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
        return m_nonsequential_improvement or m_best_improvement > 0;
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

