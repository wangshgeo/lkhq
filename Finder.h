#pragma once

#include "BrokenEdge.h"
#include "Tour.h"
#include "point_quadtree/Node.h"
#include "primitives.h"

#include <vector>

class Finder
{
public:
    Finder(const point_quadtree::Node& root, Tour& tour)
        : m_root(root), m_tour(tour) {}

    // returns true if an improving swap was found.
    bool find_best();

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

private:
    const point_quadtree::Node& m_root;
    Tour& m_tour;
    size_t m_kmax {4};
    bool m_first_improvement {true};
    bool m_save_lateral_moves {false};
    bool m_save_nonsequential {false};

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
};

