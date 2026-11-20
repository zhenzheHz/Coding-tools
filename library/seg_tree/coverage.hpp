#ifndef ZZ_LIBRARY_SEG_TREE_COVERAGE_HPP
#define ZZ_LIBRARY_SEG_TREE_COVERAGE_HPP

#include "core.hpp"

namespace zz_library {

// Segment tree over compressed elementary intervals. Leaf i represents
// [coordinates[i], coordinates[i + 1]).
template <class T>
class Segment_tree<T, coverage> {
private:
    std::vector<T> coordinates;
    std::vector<int> cover_count;
    std::vector<T> covered_length;
    int interval_count = 0;

    void update_rec(int node, int left, int right, int query_left, int query_right, int delta) {
        if (query_left <= left && right <= query_right) {
            cover_count[static_cast<std::size_t>(node)] += delta;
        } else {
            const int middle = left + (right - left) / 2;
            if (query_left <= middle) {
                update_rec(node * 2, left, middle, query_left, query_right, delta);
            }
            if (query_right > middle) {
                update_rec(node * 2 + 1, middle + 1, right, query_left, query_right, delta);
            }
        }

        const std::size_t index = static_cast<std::size_t>(node);
        if (cover_count[index] > 0) {
            covered_length[index] = coordinates[static_cast<std::size_t>(right + 1)]
                                  - coordinates[static_cast<std::size_t>(left)];
        } else if (left == right) {
            covered_length[index] = T();
        } else {
            covered_length[index] = covered_length[static_cast<std::size_t>(node * 2)]
                                  + covered_length[static_cast<std::size_t>(node * 2 + 1)];
        }
    }

public:
    Segment_tree() = default;

    explicit Segment_tree(const std::vector<T>& boundaries) {
        init(boundaries);
    }

    void init(const std::vector<T>& boundaries) {
#ifdef ZZ_LIBRARY_DEBUG
        for (std::size_t i = 1; i < boundaries.size(); ++i) {
            if (!(boundaries[i - 1] < boundaries[i])) {
                throw std::invalid_argument("zz_library::Segment_tree< T, coverage >::init: boundaries must be strictly increasing");
            }
        }
#endif
        coordinates = boundaries;
        interval_count = boundaries.empty() ? 0 : static_cast<int>(boundaries.size()) - 1;
        cover_count.assign(static_cast<std::size_t>(4) * interval_count, 0);
        covered_length.assign(static_cast<std::size_t>(4) * interval_count, T());
    }

    // Add delta coverage to compressed elementary intervals [left, right].
    void update(int left, int right, int delta) {
#ifdef ZZ_LIBRARY_DEBUG
        if (left < 0 || right >= interval_count || left > right) {
            throw std::out_of_range("zz_library::Segment_tree< T, coverage >::update: invalid interval");
        }
#endif
        if (interval_count == 0 || left > right) return;
        update_rec(1, 0, interval_count - 1, left, right, delta);
    }

    T query_covered_length() const {
        return interval_count == 0 ? T() : covered_length[1];
    }

    int size() const {
        return interval_count;
    }

    bool empty() const {
        return interval_count == 0;
    }
};

} // namespace zz_library

#endif // ZZ_LIBRARY_SEG_TREE_COVERAGE_HPP
