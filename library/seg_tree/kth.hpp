#ifndef ZZ_LIBRARY_SEG_TREE_KTH_HPP
#define ZZ_LIBRARY_SEG_TREE_KTH_HPP

#include "core.hpp"

namespace zz_library {

template <class T>
class Segment_tree<T, kth> {
private:
    struct Node {
        int left_child = 0;
        int right_child = 0;
        int count = 0;
        int minimum_last_position = -1;
    };

    int n = 0;
    std::vector<T> coordinates;
    std::vector<Node> nodes;
    std::vector<int> roots;
    std::vector<T> prefix_sum;

#ifdef ZZ_LIBRARY_DEBUG
    void check_range(int left, int right) const {
        if (left < 0 || right >= n || left > right) {
            throw std::out_of_range("zz_library::Segment_tree< T, kth >: invalid range");
        }
    }

    void check_order(int order, int length) const {
        if (order < 1 || order > length) {
            throw std::out_of_range("zz_library::Segment_tree< T, kth >: k out of range");
        }
    }
#endif

    int clone_node(int node) {
        const Node copy = nodes[static_cast<std::size_t>(node)];
        nodes.push_back(copy);
        return static_cast<int>(nodes.size()) - 1;
    }

    void pull(int node) {
        const int left_child = nodes[static_cast<std::size_t>(node)].left_child;
        const int right_child = nodes[static_cast<std::size_t>(node)].right_child;
        Node& current = nodes[static_cast<std::size_t>(node)];
        const Node& left = nodes[static_cast<std::size_t>(left_child)];
        const Node& right = nodes[static_cast<std::size_t>(right_child)];
        current.count = left.count + right.count;
        current.minimum_last_position =
            std::min(left.minimum_last_position, right.minimum_last_position);
    }

    int insert(int old_root, int left, int right, int position, int array_index) {
        const int node = clone_node(old_root);
        if (left == right) {
            Node& current = nodes[static_cast<std::size_t>(node)];
            ++current.count;
            current.minimum_last_position = array_index;
            return node;
        }

        const int middle = left + (right - left) / 2;
        if (position <= middle) {
            const int old_child = nodes[static_cast<std::size_t>(node)].left_child;
            const int new_child = insert(old_child, left, middle, position, array_index);
            nodes[static_cast<std::size_t>(node)].left_child = new_child;
        } else {
            const int old_child = nodes[static_cast<std::size_t>(node)].right_child;
            const int new_child = insert(old_child, middle + 1, right, position, array_index);
            nodes[static_cast<std::size_t>(node)].right_child = new_child;
        }
        pull(node);
        return node;
    }

    int range_count(int right_root, int left_root, int left, int right, int query_left, int query_right) const {
        if (query_right < left || right < query_left) return 0;
        if (query_left <= left && right <= query_right) {
            return nodes[static_cast<std::size_t>(right_root)].count - nodes[static_cast<std::size_t>(left_root)].count;
        }
        const int middle = left + (right - left) / 2;
        return range_count(nodes[static_cast<std::size_t>(right_root)].left_child, nodes[static_cast<std::size_t>(left_root)].left_child, left, middle, query_left, query_right) +
               range_count(nodes[static_cast<std::size_t>(right_root)].right_child, nodes[static_cast<std::size_t>(left_root)].right_child, middle + 1, right, query_left, query_right);
    }

    int find_order_rank(int right_root, int left_root, int left, int right, int order) const {
        if (left == right) return left;
        const int right_left_child = nodes[static_cast<std::size_t>(right_root)].left_child;
        const int left_left_child = nodes[static_cast<std::size_t>(left_root)].left_child;
        const int left_count = nodes[static_cast<std::size_t>(right_left_child)].count - nodes[static_cast<std::size_t>(left_left_child)].count;
        const int middle = left + (right - left) / 2;
        if (order <= left_count) {
            return find_order_rank(right_left_child, left_left_child, left, middle, order);
        }
        return find_order_rank(nodes[static_cast<std::size_t>(right_root)].right_child, nodes[static_cast<std::size_t>(left_root)].right_child, middle + 1, right, order - left_count);
    }

    int find_mex_rank(int root, int left, int right, int query_left, int query_right, int first_missing_position) const {
        if (query_right < left || right < query_left) return -1;
        const Node& current = nodes[static_cast<std::size_t>(root)];
        if (query_left <= left && right <= query_right && current.minimum_last_position >= first_missing_position) {
            return -1;
        }
        if (left == right) {
            return current.minimum_last_position < first_missing_position ? left : -1;
        }
        const int middle = left + (right - left) / 2;
        const int left_result = find_mex_rank(current.left_child, left, middle, query_left, query_right, first_missing_position);
        if (left_result != -1) return left_result;
        return find_mex_rank(current.right_child, middle + 1, right, query_left, query_right, first_missing_position);
    }

    T find_kmin_unchecked(int left, int right, int order) const {
        const int rank = find_order_rank(roots[static_cast<std::size_t>(right + 1)], roots[static_cast<std::size_t>(left)], 0, static_cast<int>(coordinates.size()) - 1, order);
        return coordinates[static_cast<std::size_t>(rank)];
    }

public:
    Segment_tree() = default;

    explicit Segment_tree(const std::vector<T>& values) {
        init(values);
    }

    void init(const std::vector<T>& values) {
        n = static_cast<int>(values.size());
        coordinates.clear();
        coordinates.reserve(values.size() + static_cast<std::size_t>(n) + 1);
        coordinates.insert(coordinates.end(), values.begin(), values.end());
        for (int value = 0; value <= n; ++value) {
            coordinates.push_back(static_cast<T>(value));
        }
        std::sort(coordinates.begin(), coordinates.end());
        coordinates.erase(std::unique(coordinates.begin(), coordinates.end()), coordinates.end());

        std::size_t levels = 1;
        std::size_t capacity = 1;
        while (capacity < coordinates.size()) {
            capacity <<= 1;
            ++levels;
        }
        nodes.clear();
        nodes.reserve(1 + static_cast<std::size_t>(n) * levels);
        nodes.emplace_back();
        nodes[0].minimum_last_position = -1;

        roots.clear();
        roots.reserve(static_cast<std::size_t>(n) + 1);
        roots.push_back(0);
        prefix_sum.assign(static_cast<std::size_t>(n) + 1, T());
        for (int i = 0; i < n; ++i) {
            const int rank = static_cast<int>(std::lower_bound(coordinates.begin(), coordinates.end(), values[static_cast<std::size_t>(i)]) - coordinates.begin());
            roots.push_back(insert(roots.back(), 0, static_cast<int>(coordinates.size()) - 1, rank, i));
            prefix_sum[static_cast<std::size_t>(i) + 1] = prefix_sum[static_cast<std::size_t>(i)] + values[static_cast<std::size_t>(i)];
        }
    }

    int size() const {
        return n;
    }

    bool empty() const {
        return n == 0;
    }

    T query_sum(int left, int right) const {
#ifdef ZZ_LIBRARY_DEBUG
        check_range(left, right);
#endif
        return prefix_sum[static_cast<std::size_t>(right) + 1] - prefix_sum[static_cast<std::size_t>(left)];
    }

    T query_min(int left, int right) const {
#ifdef ZZ_LIBRARY_DEBUG
        check_range(left, right);
#endif
        return find_kmin_unchecked(left, right, 1);
    }

    T query_max(int left, int right) const {
#ifdef ZZ_LIBRARY_DEBUG
        check_range(left, right);
#endif
        return find_kmin_unchecked(left, right, right - left + 1);
    }

    T find_kmin(int left, int right, int order) const {
#ifdef ZZ_LIBRARY_DEBUG
        check_range(left, right);
        check_order(order, right - left + 1);
#endif
        return find_kmin_unchecked(left, right, order);
    }

    T find_kmax(int left, int right, int order) const {
#ifdef ZZ_LIBRARY_DEBUG
        check_range(left, right);
        check_order(order, right - left + 1);
#endif
        return find_kmin_unchecked(left, right, right - left + 2 - order);
    }

    int count(int left, int right, const T& lower_value, const T& upper_value) const {
#ifdef ZZ_LIBRARY_DEBUG
        check_range(left, right);
#endif
        if (upper_value < lower_value) return 0;
        const int query_left = static_cast<int>(std::lower_bound(coordinates.begin(), coordinates.end(), lower_value) - coordinates.begin());
        const int query_right = static_cast<int>(std::upper_bound(coordinates.begin(), coordinates.end(), upper_value) - coordinates.begin()) - 1;
        if (query_left > query_right) return 0;
        return range_count(roots[static_cast<std::size_t>(right + 1)], roots[static_cast<std::size_t>(left)], 0, static_cast<int>(coordinates.size()) - 1, query_left, query_right);
    }

    T query_mex(int left, int right) const {
#ifdef ZZ_LIBRARY_DEBUG
        check_range(left, right);
#endif
        const int query_left = static_cast<int>(std::lower_bound(coordinates.begin(), coordinates.end(), T()) - coordinates.begin());
        const int query_right = static_cast<int>(std::upper_bound(coordinates.begin(), coordinates.end(), static_cast<T>(n)) - coordinates.begin()) - 1;
        const int rank = find_mex_rank(roots[static_cast<std::size_t>(right + 1)], 0, static_cast<int>(coordinates.size()) - 1, query_left, query_right, left);
        return coordinates[static_cast<std::size_t>(rank)];
    }
};

} // namespace zz_library

#endif // ZZ_LIBRARY_SEG_TREE_KTH_HPP
