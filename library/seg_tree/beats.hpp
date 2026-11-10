#ifndef ZZ_LIBRARY_SEG_TREE_BEATS_HPP
#define ZZ_LIBRARY_SEG_TREE_BEATS_HPP

#include "core.hpp"

namespace zz_library {

template <class T>
class Segment_tree<T, beats> {
private:
    struct Node {
        T sum{};
        T minimum{};
        T second_minimum{};
        T maximum{};
        T second_maximum{};
        T add_tag{};
        T assign_tag{};
        int minimum_count = 0;
        int maximum_count = 0;
        bool has_second_minimum = false;
        bool has_second_maximum = false;
        bool has_assignment = false;
    };

    struct Info {
        T sum{};
        T minimum{};
        T second_minimum{};
        T maximum{};
        T second_maximum{};
        int minimum_count = 0;
        int maximum_count = 0;
        bool has_second_minimum = false;
        bool has_second_maximum = false;
        bool valid = false;
    };

    int n = 0;
    std::vector<Node> tree;

#ifdef ZZ_LIBRARY_DEBUG
    void check_range(int left, int right) const {
        if (left < 0 || right >= n || left > right) {
            throw std::out_of_range("zz_library::Segment_tree< T, beats >: invalid range");
        }
    }

    void check_index(int index) const {
        if (index < 0 || index >= n) {
            throw std::out_of_range("zz_library::Segment_tree< T, beats >: index out of range");
        }
    }
#endif

    void build(int node, int left, int right, const std::vector<T>& values) {
        if (left == right) {
            Node& current = tree[static_cast<std::size_t>(node)];
            current.sum = values[static_cast<std::size_t>(left)];
            current.minimum = values[static_cast<std::size_t>(left)];
            current.maximum = values[static_cast<std::size_t>(left)];
            current.minimum_count = 1;
            current.maximum_count = 1;
            return;
        }
        const int middle = left + (right - left) / 2;
        build(node << 1, left, middle, values);
        build(node << 1 | 1, middle + 1, right, values);
        pull(node);
    }

    static Info get_info(const Node& node) {
        return {node.sum,
                node.minimum,
                node.second_minimum,
                node.maximum,
                node.second_maximum,
                node.minimum_count,
                node.maximum_count,
                node.has_second_minimum,
                node.has_second_maximum,
                true};
    }

    static Info merge_info(const Info& left, const Info& right) {
        if (!left.valid) return right;
        if (!right.valid) return left;

        Info result;
        result.valid = true;
        result.sum = left.sum + right.sum;

        if (left.minimum < right.minimum) {
            result.minimum = left.minimum;
            result.minimum_count = left.minimum_count;
            result.second_minimum = right.minimum;
            result.has_second_minimum = true;
            if (left.has_second_minimum && left.second_minimum < result.second_minimum) {
                result.second_minimum = left.second_minimum;
            }
        } else if (right.minimum < left.minimum) {
            result.minimum = right.minimum;
            result.minimum_count = right.minimum_count;
            result.second_minimum = left.minimum;
            result.has_second_minimum = true;
            if (right.has_second_minimum && right.second_minimum < result.second_minimum) {
                result.second_minimum = right.second_minimum;
            }
        } else {
            result.minimum = left.minimum;
            result.minimum_count = left.minimum_count + right.minimum_count;
            if (left.has_second_minimum || right.has_second_minimum) {
                result.has_second_minimum = true;
                if (!right.has_second_minimum ||
                    (left.has_second_minimum && left.second_minimum < right.second_minimum)) {
                    result.second_minimum = left.second_minimum;
                } else {
                    result.second_minimum = right.second_minimum;
                }
            }
        }

        if (left.maximum > right.maximum) {
            result.maximum = left.maximum;
            result.maximum_count = left.maximum_count;
            result.second_maximum = right.maximum;
            result.has_second_maximum = true;
            if (left.has_second_maximum && left.second_maximum > result.second_maximum) {
                result.second_maximum = left.second_maximum;
            }
        } else if (right.maximum > left.maximum) {
            result.maximum = right.maximum;
            result.maximum_count = right.maximum_count;
            result.second_maximum = left.maximum;
            result.has_second_maximum = true;
            if (right.has_second_maximum && right.second_maximum > result.second_maximum) {
                result.second_maximum = right.second_maximum;
            }
        } else {
            result.maximum = left.maximum;
            result.maximum_count = left.maximum_count + right.maximum_count;
            if (left.has_second_maximum || right.has_second_maximum) {
                result.has_second_maximum = true;
                if (!right.has_second_maximum ||
                    (left.has_second_maximum && left.second_maximum > right.second_maximum)) {
                    result.second_maximum = left.second_maximum;
                } else {
                    result.second_maximum = right.second_maximum;
                }
            }
        }
        return result;
    }

    void pull(int node) {
        const Info merged = merge_info(get_info(tree[node << 1]), get_info(tree[node << 1 | 1]));
        Node& current = tree[static_cast<std::size_t>(node)];
        current.sum = merged.sum;
        current.minimum = merged.minimum;
        current.second_minimum = merged.second_minimum;
        current.minimum_count = merged.minimum_count;
        current.has_second_minimum = merged.has_second_minimum;
        current.maximum = merged.maximum;
        current.second_maximum = merged.second_maximum;
        current.maximum_count = merged.maximum_count;
        current.has_second_maximum = merged.has_second_maximum;
    }

    void apply_assign(int node, int length, const T& value) {
        Node& current = tree[static_cast<std::size_t>(node)];
        current.sum = value * T(length);
        current.minimum = value;
        current.maximum = value;
        current.minimum_count = length;
        current.maximum_count = length;
        current.has_second_minimum = false;
        current.has_second_maximum = false;
        current.add_tag = T();
        current.assign_tag = value;
        current.has_assignment = true;
    }

    void apply_add(int node, int length, const T& value) {
        Node& current = tree[static_cast<std::size_t>(node)];
        current.sum += value * T(length);
        current.minimum += value;
        current.maximum += value;
        if (current.has_second_minimum) current.second_minimum += value;
        if (current.has_second_maximum) current.second_maximum += value;
        if (current.has_assignment) current.assign_tag += value;
        else current.add_tag += value;
    }

    void apply_chmin(int node, const T& value) {
        Node& current = tree[static_cast<std::size_t>(node)];
        if (current.minimum == current.maximum) {
            current.minimum = value;
        } else if (current.has_second_minimum && current.second_minimum == current.maximum) {
            current.second_minimum = value;
        }
        current.sum += (value - current.maximum) * T(current.maximum_count);
        current.maximum = value;
        if (current.has_assignment) current.assign_tag = value;
    }

    void apply_chmax(int node, const T& value) {
        Node& current = tree[static_cast<std::size_t>(node)];
        if (current.minimum == current.maximum) {
            current.maximum = value;
        } else if (current.has_second_maximum && current.second_maximum == current.minimum) {
            current.second_maximum = value;
        }
        current.sum += (value - current.minimum) * T(current.minimum_count);
        current.minimum = value;
        if (current.has_assignment) current.assign_tag = value;
    }

    void push(int node, int left, int right) {
        if (left == right) return;
        const int middle = left + (right - left) / 2;
        const int left_child = node << 1;
        const int right_child = left_child | 1;
        const int left_length = middle - left + 1;
        const int right_length = right - middle;
        const Node& current = tree[static_cast<std::size_t>(node)];

        if (current.has_assignment) {
            const T value = current.assign_tag;
            apply_assign(left_child, left_length, value);
            apply_assign(right_child, right_length, value);
        } else if (current.add_tag != T()) {
            const T value = current.add_tag;
            apply_add(left_child, left_length, value);
            apply_add(right_child, right_length, value);
        }

        // A node does not store pending clamp tags. Its extrema bound the children.
        const T parent_minimum = current.minimum;
        const T parent_maximum = current.maximum;
        tree[static_cast<std::size_t>(node)].has_assignment = false;
        tree[static_cast<std::size_t>(node)].assign_tag = T();
        tree[static_cast<std::size_t>(node)].add_tag = T();

        if (tree[static_cast<std::size_t>(left_child)].maximum > parent_maximum) {
            apply_chmin(left_child, parent_maximum);
        }
        if (tree[static_cast<std::size_t>(right_child)].maximum > parent_maximum) {
            apply_chmin(right_child, parent_maximum);
        }
        if (tree[static_cast<std::size_t>(left_child)].minimum < parent_minimum) {
            apply_chmax(left_child, parent_minimum);
        }
        if (tree[static_cast<std::size_t>(right_child)].minimum < parent_minimum) {
            apply_chmax(right_child, parent_minimum);
        }
    }

    void range_add(int node, int left, int right, int query_left, int query_right, const T& value) {
        if (query_left <= left && right <= query_right) {
            apply_add(node, right - left + 1, value);
            return;
        }
        push(node, left, right);
        const int middle = left + (right - left) / 2;
        if (query_left <= middle) {
            range_add(node << 1, left, middle, query_left, query_right, value);
        }
        if (query_right > middle) {
            range_add(node << 1 | 1, middle + 1, right, query_left, query_right, value);
        }
        pull(node);
    }

    void range_assign(int node, int left, int right, int query_left, int query_right, const T& value) {
        if (query_left <= left && right <= query_right) {
            apply_assign(node, right - left + 1, value);
            return;
        }
        push(node, left, right);
        const int middle = left + (right - left) / 2;
        if (query_left <= middle) {
            range_assign(node << 1, left, middle, query_left, query_right, value);
        }
        if (query_right > middle) {
            range_assign(node << 1 | 1, middle + 1, right, query_left, query_right, value);
        }
        pull(node);
    }

    void range_chmin(int node, int left, int right, int query_left, int query_right, const T& value) {
        const Node& current = tree[static_cast<std::size_t>(node)];
        if (current.maximum <= value) return;
        if (query_left <= left && right <= query_right &&
            (!current.has_second_maximum || current.second_maximum < value)) {
            apply_chmin(node, value);
            return;
        }
        push(node, left, right);
        const int middle = left + (right - left) / 2;
        if (query_left <= middle) {
            range_chmin(node << 1, left, middle, query_left, query_right, value);
        }
        if (query_right > middle) {
            range_chmin(node << 1 | 1, middle + 1, right, query_left, query_right, value);
        }
        pull(node);
    }

    void range_chmax(int node, int left, int right, int query_left, int query_right, const T& value) {
        const Node& current = tree[static_cast<std::size_t>(node)];
        if (current.minimum >= value) return;
        if (query_left <= left && right <= query_right &&
            (!current.has_second_minimum || current.second_minimum > value)) {
            apply_chmax(node, value);
            return;
        }
        push(node, left, right);
        const int middle = left + (right - left) / 2;
        if (query_left <= middle) {
            range_chmax(node << 1, left, middle, query_left, query_right, value);
        }
        if (query_right > middle) {
            range_chmax(node << 1 | 1, middle + 1, right, query_left, query_right, value);
        }
        pull(node);
    }

    Info query_info(int node, int left, int right, int query_left, int query_right) {
        if (query_left <= left && right <= query_right) return get_info(tree[node]);
        push(node, left, right);
        const int middle = left + (right - left) / 2;
        if (query_right <= middle) {
            return query_info(node << 1, left, middle, query_left, query_right);
        }
        if (query_left > middle) {
            return query_info(node << 1 | 1, middle + 1, right, query_left, query_right);
        }
        return merge_info(query_info(node << 1, left, middle, query_left, query_right), query_info(node << 1 | 1, middle + 1, right, query_left, query_right));
    }

public:
    Segment_tree() = default;

    explicit Segment_tree(const std::vector<T>& values) {
        init(values);
    }

    void init(const std::vector<T>& values) {
        n = static_cast<int>(values.size());
        tree.assign(static_cast<std::size_t>(std::max(1, n * 4)), Node{});
        if (n) build(1, 0, n - 1, values);
    }

    int size() const {
        return n;
    }

    bool empty() const {
        return n == 0;
    }

    void chmin(int left, int right, const T& value) {
#ifdef ZZ_LIBRARY_DEBUG
        check_range(left, right);
#endif
        range_chmin(1, 0, n - 1, left, right, value);
    }

    void chmax(int left, int right, const T& value) {
#ifdef ZZ_LIBRARY_DEBUG
        check_range(left, right);
#endif
        range_chmax(1, 0, n - 1, left, right, value);
    }

    void set(int index, const T& value) {
#ifdef ZZ_LIBRARY_DEBUG
        check_index(index);
#endif
        range_assign(1, 0, n - 1, index, index, value);
    }

    void add(int index, const T& value) {
#ifdef ZZ_LIBRARY_DEBUG
        check_index(index);
#endif
        if (value == T()) return;
        range_add(1, 0, n - 1, index, index, value);
    }

    void update(int left, int right, const T& value) {
#ifdef ZZ_LIBRARY_DEBUG
        check_range(left, right);
#endif
        if (value == T()) return;
        range_add(1, 0, n - 1, left, right, value);
    }

    void assign(int left, int right, const T& value) {
#ifdef ZZ_LIBRARY_DEBUG
        check_range(left, right);
#endif
        range_assign(1, 0, n - 1, left, right, value);
    }

    T query_sum(int left, int right) {
#ifdef ZZ_LIBRARY_DEBUG
        check_range(left, right);
#endif
        return query_info(1, 0, n - 1, left, right).sum;
    }

    T query_min(int left, int right) {
#ifdef ZZ_LIBRARY_DEBUG
        check_range(left, right);
#endif
        return query_info(1, 0, n - 1, left, right).minimum;
    }

    T query_max(int left, int right) {
#ifdef ZZ_LIBRARY_DEBUG
        check_range(left, right);
#endif
        return query_info(1, 0, n - 1, left, right).maximum;
    }

    int max_count(int left, int right) {
#ifdef ZZ_LIBRARY_DEBUG
        check_range(left, right);
#endif
        return query_info(1, 0, n - 1, left, right).maximum_count;
    }

    int min_count(int left, int right) {
#ifdef ZZ_LIBRARY_DEBUG
        check_range(left, right);
#endif
        return query_info(1, 0, n - 1, left, right).minimum_count;
    }

    std::optional<T> second_max(int left, int right) {
#ifdef ZZ_LIBRARY_DEBUG
        check_range(left, right);
#endif
        const Info info = query_info(1, 0, n - 1, left, right);
        if (!info.has_second_maximum) return std::nullopt;
        return info.second_maximum;
    }

    std::optional<T> second_min(int left, int right) {
#ifdef ZZ_LIBRARY_DEBUG
        check_range(left, right);
#endif
        const Info info = query_info(1, 0, n - 1, left, right);
        if (!info.has_second_minimum) return std::nullopt;
        return info.second_minimum;
    }
};

} // namespace zz_library

#endif // ZZ_LIBRARY_SEG_TREE_BEATS_HPP
