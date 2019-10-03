#include "exchange_pair.hh"

namespace merge {

int ExchangePair::compute_improvement(const std::vector<primitives::space_t> &x, const std::vector<primitives::space_t> &y) {
    if (improvement) {
        return *improvement;
    }
    LengthCalculator calc(x, y);
    improvement = std::make_optional<int>(0);
    // note that each edge has 2 entries in the map.
    for (const auto &pair : current.map()) {
        *improvement += cost(pair.second, calc);
    }
    for (const auto &pair : candidate.map()) {
        *improvement -= cost(pair.second, calc);
    }
    if ((*improvement & 1) == 1) { // odd
        throw std::logic_error("2x the improvement is an odd number.");
    }
    *improvement /= 2;
    return *improvement;
}

}  // namespace merge
