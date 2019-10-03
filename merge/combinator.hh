#pragma once

#include <vector>

#include "cycle_util.hh"
#include "exchange_pair.hh"

namespace merge {

class Combinator {
 public:
    // Exchange pairs are sorted by improvement, highest to lowest.
    Combinator(const std::vector<ExchangePair> &sorted_exchange_pairs, const Tour &best_tour, const Tour &candidate_tour)
        : exchange_pairs_(sorted_exchange_pairs), best_tour_(best_tour), candidate_tour_(candidate_tour) {}

    void find(size_t i = 0) {
        if (i == exchange_pairs_.size()) {
            return;
        }
        const auto improvement = *exchange_pairs_[i].improvement;
        if (margin_ + improvement <= 0) {
            return;
        }
        combo_.push_back(i);
        margin_ += improvement;
        print_combo();
        find(i + 1); // try with this.
        combo_.pop_back();
        margin_ -= improvement;
        find(i + 1); // try without this.
    }

    size_t combo_count() const { return combo_count_; }

 private:
    const std::vector<ExchangePair> &exchange_pairs_;
    const Tour &best_tour_;
    const Tour &candidate_tour_;

    std::vector<size_t> combo_;
    int margin_{0};
    size_t combo_count_{0};

    void print_combo() {
        const auto breaks_cycle = cycle_util::breaks_cycle(best_tour_, candidate_tour_, exchange_pairs_, combo_);
        if (breaks_cycle) {
            return;
        }
        throw std::logic_error("found!");
        ++combo_count_;
        for (const auto &i : combo_) {
            std::cout << i << " ";
        }
        std::cout << std::endl;
    }
};

}  // namespace merge
