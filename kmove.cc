#include "kmove.hh"

KMove KMove::make_reverse() const {
    KMove reverse = *this;
    // TODO: need to record removal ends.
    return reverse;
}

void KMove::validate() const {
    if (starts.size() != ends.size() or starts.size() != removes.size()) {
        throw std::logic_error("invalid kmove.");
    }
}

KMove KMove::operator+(const KMove& other) const {
    auto new_kmove = *this;
    new_kmove.starts.insert(std::end(new_kmove.starts)
        , std::cbegin(other.starts)
        , std::cend(other.starts));
    new_kmove.ends.insert(std::end(new_kmove.ends)
        , std::cbegin(other.ends)
        , std::cend(other.ends));
    new_kmove.removes.insert(std::end(new_kmove.removes)
        , std::cbegin(other.removes)
        , std::cend(other.removes));
    return new_kmove;
}

size_t KMove::current_k() const { return starts.size(); }

bool KMove::removable(primitives::point_id_t i) const {
    return not contains(removes, i);
}

bool KMove::startable(primitives::point_id_t i) const {
    return std::count(std::cbegin(starts), std::cend(starts), i) < 2;
}

bool KMove::endable(primitives::point_id_t i) const {
    return std::count(std::cbegin(ends), std::cend(ends), i) < 2;
}

void KMove::clear() {
    starts.clear();
    ends.clear();
    removes.clear();
}

bool KMove::contains(const std::vector<primitives::point_id_t>& points
    , primitives::point_id_t point) {
    return std::find(std::cbegin(points), std::cend(points), point) != std::cend(points);
}

