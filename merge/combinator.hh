#pragma once

#include <vector>
#include <optional>

#include "cycle_util.hh"
#include "exchange_pair.hh"
#include <debug_util.hh>

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
        check_combo();
        find(i + 1); // try with this.
        combo_.pop_back();
        margin_ -= improvement;
        find(i + 1); // try without this.
    }

    size_t viable_count() const { return viable_count_; }
    const auto &best_combo() const { return best_combo_; }
    const auto &best_improvement() const { return best_improvement_; }
    const auto &checks() const { return checks_; }

 private:
    const std::vector<ExchangePair> &exchange_pairs_;
    const Tour &best_tour_;
    const Tour &candidate_tour_;

    using Combo = std::vector<size_t>;
    Combo combo_;
    std::optional<int> best_improvement_;
    std::optional<Combo> best_combo_;
    int margin_{0};
    size_t viable_count_{0};
    size_t checks_{0};

    void check_combo() {
        ++checks_;
        const auto breaks_cycle = cycle_util::breaks_cycle(best_tour_, candidate_tour_, exchange_pairs_, combo_);
        if (breaks_cycle) {
            if (cycle_util::count_cycles(best_tour_, candidate_tour_, exchange_pairs_, combo_) == 2) {
                std::cout << "found double cycle tour with margin " << margin_ << std::endl;
            }
            return;
        }
        ++viable_count_;
        if (not best_improvement_ or margin_ > *best_improvement_) {
            best_combo_ = std::make_optional<Combo>(combo_);
            best_improvement_ = std::make_optional(margin_);
            std::cout << "better combo: ";
            for (const auto &i : *best_combo_) {
                std::cout << i << " ";
            }
            std::cout << std::endl;
        }
    }
};

}  // namespace merge
