#ifndef ZZ_LIBRARY_SEG_TREE_ZKW_HPP
#define ZZ_LIBRARY_SEG_TREE_ZKW_HPP

#include "core.hpp"

namespace zz_library {

template <class T>
class Segment_tree<T, zkw> {
private:
    int n = 0;
    int tree_size = 1;
    int height = 0;

    std::vector<T> tree_min;
    std::vector<T> tree_max;
    std::vector<T> tree_sum;
    // GCD uses gcd(first value, adjacent differences), which remains compatible
    // with lazy range addition because adding a constant preserves differences.
    std::vector<T> first_value;
    std::vector<T> last_value;
    std::vector<T> diff_gcd;
    std::vector<T> lazy_add;
    std::vector<T> lazy_assign;
    std::vector<int> segment_length;
    std::vector<int> min_index;
    std::vector<int> max_index;
    std::vector<int> first_index;
    std::vector<unsigned char> has_assignment;

    static T absolute(T value) {
        return value < T() ? -value : value;
    }

    static T gcd(T left, T right) {
        left = absolute(left);
        right = absolute(right);
        while (right != T()) {
            T remainder = left % right;
            left = right;
            right = remainder;
        }
        return left;
    }

#ifdef ZZ_LIBRARY_DEBUG
    void check_range(int left, int right) const {
        if (left < 0 || right >= n || left > right) {
            throw std::out_of_range("zz_library::Segment_tree< T, zkw >: invalid range");
        }
    }

    void check_index(int index) const {
        if (index < 0 || index >= n) {
            throw std::out_of_range("zz_library::Segment_tree< T, zkw >: index out of range");
        }
    }
#endif

    void apply_assign(int node, const T& value) {
        if (!segment_length[node]) return;
        tree_min[node] = value;
        tree_max[node] = value;
        tree_sum[node] = value * T(segment_length[node]);
        first_value[node] = value;
        last_value[node] = value;
        diff_gcd[node] = T();
        min_index[node] = first_index[node];
        max_index[node] = first_index[node];
        lazy_assign[node] = value;
        lazy_add[node] = T();
        has_assignment[node] = 1;
    }

    void apply_add(int node, const T& value) {
        if (!segment_length[node]) return;
        tree_min[node] += value;
        tree_max[node] += value;
        tree_sum[node] += value * T(segment_length[node]);
        first_value[node] += value;
        last_value[node] += value;
        if (has_assignment[node]) lazy_assign[node] += value;
        else lazy_add[node] += value;
    }

    void push_down(int node) {
        if (has_assignment[node]) {
            apply_assign(node << 1, lazy_assign[node]);
            apply_assign(node << 1 | 1, lazy_assign[node]);
            has_assignment[node] = 0;
        }

        if (lazy_add[node] != T()) {
            apply_add(node << 1, lazy_add[node]);
            apply_add(node << 1 | 1, lazy_add[node]);
            lazy_add[node] = T();
        }
    }

    void pull_up(int node) {
        const int left = node << 1;
        const int right = left | 1;

        if (!segment_length[left]) {
            tree_min[node] = tree_min[right];
            tree_max[node] = tree_max[right];
            tree_sum[node] = tree_sum[right];
            first_value[node] = first_value[right];
            last_value[node] = last_value[right];
            diff_gcd[node] = diff_gcd[right];
            min_index[node] = min_index[right];
            max_index[node] = max_index[right];
            first_index[node] = first_index[right];
        } else if (!segment_length[right]) {
            tree_min[node] = tree_min[left];
            tree_max[node] = tree_max[left];
            tree_sum[node] = tree_sum[left];
            first_value[node] = first_value[left];
            last_value[node] = last_value[left];
            diff_gcd[node] = diff_gcd[left];
            min_index[node] = min_index[left];
            max_index[node] = max_index[left];
            first_index[node] = first_index[left];
        } else {
            tree_sum[node] = tree_sum[left] + tree_sum[right];
            first_value[node] = first_value[left];
            last_value[node] = last_value[right];
            diff_gcd[node] = gcd(gcd(diff_gcd[left], diff_gcd[right]),
                                 absolute(last_value[left] - first_value[right]));
            first_index[node] = first_index[left];

            if (tree_min[left] <= tree_min[right]) {
                tree_min[node] = tree_min[left];
                min_index[node] = min_index[left];
            } else {
                tree_min[node] = tree_min[right];
                min_index[node] = min_index[right];
            }

            if (tree_max[left] >= tree_max[right]) {
                tree_max[node] = tree_max[left];
                max_index[node] = max_index[left];
            } else {
                tree_max[node] = tree_max[right];
                max_index[node] = max_index[right];
            }
        }
    }

    void push_path(int node) {
        for (int level = height; level >= 1; --level) {
            push_down(node >> level);
        }
    }

    void pull_path(int left_node, int right_node) {
        for (int level = 1; level <= height; ++level) {
            if (((left_node >> level) << level) != left_node) {
                pull_up(left_node >> level);
            }
            if (((right_node >> level) << level) != right_node) {
                pull_up((right_node - 1) >> level);
            }
        }
    }

    template <bool Assignment>
    void range_update(int left, int right, const T& value) {
#ifdef ZZ_LIBRARY_DEBUG
        check_range(left, right);
#endif

        int left_node = left + tree_size;
        int right_node = right + 1 + tree_size;
        const int original_left = left_node;
        const int original_right = right_node;

        push_path(left_node);
        push_path(right_node - 1);

        while (left_node < right_node) {
            if (left_node & 1) {
                if constexpr (Assignment) apply_assign(left_node, value);
                else apply_add(left_node, value);
                ++left_node;
            }
            if (right_node & 1) {
                --right_node;
                if constexpr (Assignment) apply_assign(right_node, value);
                else apply_add(right_node, value);
            }
            left_node >>= 1;
            right_node >>= 1;
        }

        pull_path(original_left, original_right);
    }

    template <class Value, class Combine>
    Value query(int left, int right, const std::vector<Value>& values, Combine combine) {
#ifdef ZZ_LIBRARY_DEBUG
        check_range(left, right);
#endif

        int left_node = left + tree_size;
        int right_node = right + 1 + tree_size;
        push_path(left_node);
        push_path(right_node - 1);

        Value result{};
        bool found = false;
        while (left_node < right_node) {
            if (left_node & 1) {
                if (!found) result = values[left_node];
                else result = combine(result, values[left_node]);
                found = true;
                ++left_node;
            }
            if (right_node & 1) {
                --right_node;
                if (!found) result = values[right_node];
                else result = combine(result, values[right_node]);
                found = true;
            }
            left_node >>= 1;
            right_node >>= 1;
        }
        return result;
    }

public:
    Segment_tree() = default;

    explicit Segment_tree(const std::vector<T>& values) {
        init(values);
    }

    void init(const std::vector<T>& values) {
        n = static_cast<int>(values.size());
        tree_size = 1;
        height = 0;
        while (tree_size < n) {
            tree_size <<= 1;
            ++height;
        }

        const int capacity = tree_size << 1;
        tree_min.assign(capacity, T());
        tree_max.assign(capacity, T());
        tree_sum.assign(capacity, T());
        first_value.assign(capacity, T());
        last_value.assign(capacity, T());
        diff_gcd.assign(capacity, T());
        lazy_add.assign(capacity, T());
        lazy_assign.assign(capacity, T());
        segment_length.assign(capacity, 0);
        min_index.assign(capacity, -1);
        max_index.assign(capacity, -1);
        first_index.assign(capacity, -1);
        has_assignment.assign(capacity, 0);

        for (int i = 0; i < n; ++i) {
            const int node = tree_size + i;
            tree_min[node] = values[static_cast<std::size_t>(i)];
            tree_max[node] = values[static_cast<std::size_t>(i)];
            tree_sum[node] = values[static_cast<std::size_t>(i)];
            first_value[node] = values[static_cast<std::size_t>(i)];
            last_value[node] = values[static_cast<std::size_t>(i)];
            segment_length[node] = 1;
            min_index[node] = i;
            max_index[node] = i;
            first_index[node] = i;
        }

        for (int node = tree_size - 1; node >= 1; --node) {
            const int left = node << 1;
            const int right = left | 1;
            segment_length[node] = segment_length[left] + segment_length[right];
            pull_up(node);
        }
    }

    void set(int index, const T& value) {
#ifdef ZZ_LIBRARY_DEBUG
        check_index(index);
#endif
        range_update<true>(index, index, value);
    }

    void add(int index, const T& value) {
#ifdef ZZ_LIBRARY_DEBUG
        check_index(index);
#endif
        range_update<false>(index, index, value);
    }

    void update(int left, int right, const T& value) {
        range_update<false>(left, right, value);
    }

    void assign(int left, int right, const T& value) {
        range_update<true>(left, right, value);
    }

    T query_min(int left, int right) {
        return query(left, right, tree_min, [](const T& a, const T& b) { return a < b ? a : b; });
    }

    T query_max(int left, int right) {
        return query(left, right, tree_max, [](const T& a, const T& b) { return a > b ? a : b; });
    }

    int where_min(int left, int right) {
#ifdef ZZ_LIBRARY_DEBUG
        check_range(left, right);
#endif
        int left_node = left + tree_size;
        int right_node = right + 1 + tree_size;
        push_path(left_node);
        push_path(right_node - 1);
        int answer = -1;
        T best{};
        while (left_node < right_node) {
            if (left_node & 1) {
                if (answer < 0 || tree_min[left_node] < best ||
                    (tree_min[left_node] == best && min_index[left_node] < answer)) {
                    answer = min_index[left_node];
                    best = tree_min[left_node];
                }
                ++left_node;
            }
            if (right_node & 1) {
                --right_node;
                if (answer < 0 || tree_min[right_node] < best ||
                    (tree_min[right_node] == best && min_index[right_node] < answer)) {
                    answer = min_index[right_node];
                    best = tree_min[right_node];
                }
            }
            left_node >>= 1;
            right_node >>= 1;
        }
        return answer;
    }

    int where_max(int left, int right) {
#ifdef ZZ_LIBRARY_DEBUG
        check_range(left, right);
#endif
        int left_node = left + tree_size;
        int right_node = right + 1 + tree_size;
        push_path(left_node);
        push_path(right_node - 1);
        int answer = -1;
        T best{};
        while (left_node < right_node) {
            if (left_node & 1) {
                if (answer < 0 || tree_max[left_node] > best ||
                    (tree_max[left_node] == best && max_index[left_node] < answer)) {
                    answer = max_index[left_node];
                    best = tree_max[left_node];
                }
                ++left_node;
            }
            if (right_node & 1) {
                --right_node;
                if (answer < 0 || tree_max[right_node] > best ||
                    (tree_max[right_node] == best && max_index[right_node] < answer)) {
                    answer = max_index[right_node];
                    best = tree_max[right_node];
                }
            }
            left_node >>= 1;
            right_node >>= 1;
        }
        return answer;
    }

    T query_sum(int left, int right) {
        return query(left, right, tree_sum, [](const T& a, const T& b) { return a + b; });
    }

    T query_gcd(int left, int right) {
#ifdef ZZ_LIBRARY_DEBUG
        check_range(left, right);
#endif
        int left_node = left + tree_size;
        int right_node = right + 1 + tree_size;
        push_path(left_node);
        push_path(right_node - 1);
        bool has_left = false;
        bool has_right = false;
        T left_first{};
        T left_last{};
        T left_diff{};
        T right_first{};
        T right_diff{};

        const auto append_left = [&](int node) {
            if (!has_left) {
                left_first = first_value[node];
                left_last = last_value[node];
                left_diff = diff_gcd[node];
                has_left = true;
            } else {
                left_diff = gcd(gcd(left_diff, diff_gcd[node]), absolute(left_last - first_value[node]));
                left_last = last_value[node];
            }
        };
        const auto prepend_right = [&](int node) {
            if (!has_right) {
                right_first = first_value[node];
                right_diff = diff_gcd[node];
                has_right = true;
            } else {
                right_diff = gcd(gcd(diff_gcd[node], right_diff), absolute(last_value[node] - right_first));
                right_first = first_value[node];
            }
        };

        while (left_node < right_node) {
            if (left_node & 1) append_left(left_node++);
            if (right_node & 1) prepend_right(--right_node);
            left_node >>= 1;
            right_node >>= 1;
        }

        T first = left_first;
        T differences = left_diff;
        if (has_right) {
            if (!has_left) first = right_first;
            else differences = gcd(gcd(left_diff, right_diff), absolute(left_last - right_first));
            if (!has_left) differences = right_diff;
        }
        return gcd(absolute(first), differences);
    }

    int size() const {
        return n;
    }

    bool empty() const {
        return n == 0;
    }
};

} // namespace zz_library

#endif // ZZ_LIBRARY_SEG_TREE_ZKW_HPP
