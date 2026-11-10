#ifndef ZZ_LIBRARY_SEG_TREE_LAZY_HPP
#define ZZ_LIBRARY_SEG_TREE_LAZY_HPP

#include "core.hpp"

namespace zz_library {

template <class T>
class Segment_tree<T, lazy> {
private:
    static constexpr int domain_left = 0;
    static constexpr int domain_right = 1'000'000'000;

    struct Node {
        T minimum{};
        T maximum{};
        T sum{};
        T first{};
        T last{};
        // gcd(values[l..r]) = gcd(abs(first), gcd(adjacent differences)).
        T diff_gcd{};
        T add_tag{};
        T assign_tag{};
        int left = 0;
        int right = 0;
        int min_index = 0;
        int max_index = 0;
        bool has_assign = false;
    };

    struct Info {
        T minimum{};
        T maximum{};
        T sum{};
        T first{};
        T last{};
        T diff_gcd{};
        int min_index = 0;
        int max_index = 0;
        bool valid = false;
    };

    std::vector<Node> nodes{Node{}};
    std::vector<int> free_nodes;
    int root = 0;

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

    static int midpoint(int left, int right) {
        return left + (right - left) / 2;
    }

    static T sum_for(const T& value, int left, int right) {
        return value * static_cast<std::int64_t>(right - left + 1);
    }

#ifdef ZZ_LIBRARY_DEBUG
    static void check_range(int left, int right) {
        if (left < domain_left || right > domain_right || left > right) {
            throw std::out_of_range("zz_library::Segment_tree< T, lazy >: invalid range");
        }
    }

    static void check_index(int index) {
        if (index < domain_left || index > domain_right) {
            throw std::out_of_range("zz_library::Segment_tree< T, lazy >: index out of range");
        }
    }
#endif

    int allocate_node() {
        if (free_nodes.empty()) {
            nodes.emplace_back();
            return static_cast<int>(nodes.size() - 1);
        }
        const int index = free_nodes.back();
        free_nodes.pop_back();
        nodes[static_cast<std::size_t>(index)] = Node{};
        return index;
    }

    void release_subtree(int index) {
        if (!index) return;
        const int left = nodes[static_cast<std::size_t>(index)].left;
        const int right = nodes[static_cast<std::size_t>(index)].right;
        release_subtree(left);
        release_subtree(right);
        nodes[static_cast<std::size_t>(index)] = Node{};
        free_nodes.push_back(index);
    }

    void apply_assign(int index, int left, int right, const T& value) {
        const int old_left = nodes[static_cast<std::size_t>(index)].left;
        const int old_right = nodes[static_cast<std::size_t>(index)].right;
        release_subtree(old_left);
        release_subtree(old_right);

        Node& node = nodes[static_cast<std::size_t>(index)];
        node.minimum = value;
        node.maximum = value;
        node.sum = sum_for(value, left, right);
        node.first = value;
        node.last = value;
        node.diff_gcd = T();
        node.left = 0;
        node.right = 0;
        node.min_index = left;
        node.max_index = left;
        node.add_tag = T();
        node.assign_tag = value;
        node.has_assign = true;
    }

    void apply_add(int index, int left, int right, const T& value) {
        Node& node = nodes[static_cast<std::size_t>(index)];
        node.minimum += value;
        node.maximum += value;
        node.sum += sum_for(value, left, right);
        node.first += value;
        node.last += value;
        if (!node.left && !node.right) {
            node.min_index = left;
            node.max_index = left;
        }
        if (node.has_assign) node.assign_tag += value;
        else node.add_tag += value;
    }

    void push(int index, int left, int right) {
        if (left == right) return;
        int child_left = nodes[static_cast<std::size_t>(index)].left;
        int child_right = nodes[static_cast<std::size_t>(index)].right;
        const int middle = midpoint(left, right);

        if (nodes[static_cast<std::size_t>(index)].has_assign) {
            const T value = nodes[static_cast<std::size_t>(index)].assign_tag;
            if (!child_left) child_left = allocate_node();
            if (!child_right) child_right = allocate_node();
            apply_assign(child_left, left, middle, value);
            apply_assign(child_right, middle + 1, right, value);
            nodes[static_cast<std::size_t>(index)].has_assign = false;
            nodes[static_cast<std::size_t>(index)].assign_tag = T();
        }

        const T add_value = nodes[static_cast<std::size_t>(index)].add_tag;
        if (add_value != T()) {
            if (!child_left) child_left = allocate_node();
            if (!child_right) child_right = allocate_node();
            apply_add(child_left, left, middle, add_value);
            apply_add(child_right, middle + 1, right, add_value);
            nodes[static_cast<std::size_t>(index)].add_tag = T();
        }

        nodes[static_cast<std::size_t>(index)].left = child_left;
        nodes[static_cast<std::size_t>(index)].right = child_right;
    }

    void pull(int index, int left, int right) {
        const int child_left = nodes[static_cast<std::size_t>(index)].left;
        const int child_right = nodes[static_cast<std::size_t>(index)].right;
        const Node& lhs = nodes[static_cast<std::size_t>(child_left)];
        const Node& rhs = nodes[static_cast<std::size_t>(child_right)];
        const int middle = midpoint(left, right);

        Node& node = nodes[static_cast<std::size_t>(index)];
        node.sum = lhs.sum + rhs.sum;
        node.first = lhs.first;
        node.last = rhs.last;
        node.diff_gcd = gcd(gcd(lhs.diff_gcd, rhs.diff_gcd), absolute(lhs.last - rhs.first));

        const T left_min = child_left ? lhs.minimum : T();
        const T right_min = child_right ? rhs.minimum : T();
        const T left_max = child_left ? lhs.maximum : T();
        const T right_max = child_right ? rhs.maximum : T();
        const int left_min_index = child_left ? lhs.min_index : left;
        const int right_min_index = child_right ? rhs.min_index : middle + 1;
        const int left_max_index = child_left ? lhs.max_index : left;
        const int right_max_index = child_right ? rhs.max_index : middle + 1;

        if (left_min <= right_min) {
            node.minimum = left_min;
            node.min_index = left_min_index;
        } else {
            node.minimum = right_min;
            node.min_index = right_min_index;
        }
        if (left_max >= right_max) {
            node.maximum = left_max;
            node.max_index = left_max_index;
        } else {
            node.maximum = right_max;
            node.max_index = right_max_index;
        }
    }

    bool can_remove(int index) const {
        const Node& node = nodes[static_cast<std::size_t>(index)];
        return !node.left && !node.right && !node.has_assign && node.add_tag == T() &&
               node.minimum == T() && node.maximum == T();
    }

    int update_add(int index, int left, int right,
                   int query_left, int query_right, const T& value) {
        if (value == T()) return index;
        if (!index) index = allocate_node();
        if (query_left <= left && right <= query_right) {
            apply_add(index, left, right, value);
            return index;
        }

        push(index, left, right);
        const int middle = midpoint(left, right);
        if (query_left <= middle) {
            const int child = update_add(nodes[static_cast<std::size_t>(index)].left, left, middle, query_left, query_right, value);
            nodes[static_cast<std::size_t>(index)].left = child;
        }
        if (query_right > middle) {
            const int child = update_add(nodes[static_cast<std::size_t>(index)].right, middle + 1, right, query_left, query_right, value);
            nodes[static_cast<std::size_t>(index)].right = child;
        }
        pull(index, left, right);
        if (can_remove(index)) {
            release_subtree(index);
            return 0;
        }
        return index;
    }

    int update_assign(int index, int left, int right, int query_left, int query_right, const T& value) {
        if (query_left <= left && right <= query_right) {
            if (value == T()) {
                release_subtree(index);
                return 0;
            }
            if (!index) index = allocate_node();
            apply_assign(index, left, right, value);
            return index;
        }
        if (!index && value == T()) return 0;
        if (!index) index = allocate_node();

        push(index, left, right);
        const int middle = midpoint(left, right);
        if (query_left <= middle) {
            const int child = update_assign(nodes[static_cast<std::size_t>(index)].left, left, middle, query_left, query_right, value);
            nodes[static_cast<std::size_t>(index)].left = child;
        }
        if (query_right > middle) {
            const int child = update_assign(nodes[static_cast<std::size_t>(index)].right, middle + 1, right, query_left, query_right, value);
            nodes[static_cast<std::size_t>(index)].right = child;
        }
        pull(index, left, right);
        if (can_remove(index)) {
            release_subtree(index);
            return 0;
        }
        return index;
    }

    int build(const std::vector<T>& values, int left, int right) {
        if (left >= static_cast<int>(values.size())) return 0;
        if (left == right) {
            const T& value = values[static_cast<std::size_t>(left)];
            if (value == T()) return 0;
            const int index = allocate_node();
            apply_assign(index, left, right, value);
            return index;
        }

        const int middle = midpoint(left, right);
        const int child_left = build(values, left, middle);
        const int child_right = build(values, middle + 1, right);
        if (!child_left && !child_right) return 0;
        const int index = allocate_node();
        nodes[static_cast<std::size_t>(index)].left = child_left;
        nodes[static_cast<std::size_t>(index)].right = child_right;
        pull(index, left, right);
        return index;
    }

    static Info merge(const Info& left, const Info& right) {
        if (!left.valid) return right;
        if (!right.valid) return left;
        Info result;
        result.valid = true;
        result.sum = left.sum + right.sum;
        result.first = left.first;
        result.last = right.last;
        result.diff_gcd = gcd(gcd(left.diff_gcd, right.diff_gcd), absolute(left.last - right.first));
        if (left.minimum <= right.minimum) {
            result.minimum = left.minimum;
            result.min_index = left.min_index;
        } else {
            result.minimum = right.minimum;
            result.min_index = right.min_index;
        }
        if (left.maximum >= right.maximum) {
            result.maximum = left.maximum;
            result.max_index = left.max_index;
        } else {
            result.maximum = right.maximum;
            result.max_index = right.max_index;
        }
        return result;
    }

    Info query_rec(int index, int left, int right, int query_left, int query_right, bool carry_assign, const T& carry_assign_value, const T& carry_add) const {
        if (query_left <= left && right <= query_right) {
            Info result;
            result.valid = true;
            if (index) {
                const Node& node = nodes[static_cast<std::size_t>(index)];
                result.minimum = node.minimum;
                result.maximum = node.maximum;
                result.sum = node.sum;
                result.first = node.first;
                result.last = node.last;
                result.diff_gcd = node.diff_gcd;
                result.min_index = node.min_index;
                result.max_index = node.max_index;
            } else {
                result.min_index = left;
                result.max_index = left;
            }
            if (carry_assign) {
                result.minimum = carry_assign_value;
                result.maximum = carry_assign_value;
                result.sum = sum_for(carry_assign_value, left, right);
                result.first = carry_assign_value;
                result.last = carry_assign_value;
                result.diff_gcd = T();
                result.min_index = left;
                result.max_index = left;
            } else if (carry_add != T()) {
                result.minimum += carry_add;
                result.maximum += carry_add;
                result.sum += sum_for(carry_add, left, right);
                result.first += carry_add;
                result.last += carry_add;
            }
            return result;
        }

        // Queries carry pending ancestor tags without pushing or allocating nodes.
        bool next_assign = carry_assign;
        T next_assign_value = carry_assign_value;
        T next_add = carry_add;
        if (!carry_assign && index) {
            const Node& node = nodes[static_cast<std::size_t>(index)];
            if (node.has_assign) {
                next_assign = true;
                next_assign_value = node.assign_tag + carry_add;
                next_add = T();
            } else {
                next_add = node.add_tag + carry_add;
            }
        }

        const int middle = midpoint(left, right);
        const int child_left = index ? nodes[static_cast<std::size_t>(index)].left : 0;
        const int child_right = index ? nodes[static_cast<std::size_t>(index)].right : 0;
        if (query_right <= middle) {
            return query_rec(child_left, left, middle, query_left, query_right, next_assign, next_assign_value, next_add);
        }
        if (query_left > middle) {
            return query_rec(child_right, middle + 1, right, query_left, query_right, next_assign, next_assign_value, next_add);
        }
        return merge(query_rec(child_left, left, middle, query_left, query_right, next_assign, next_assign_value, next_add),
                     query_rec(child_right, middle + 1, right, query_left, query_right, next_assign, next_assign_value, next_add));
    }

    Info query_info(int left, int right) const {
#ifdef ZZ_LIBRARY_DEBUG
        check_range(left, right);
#endif
        return query_rec(root, domain_left, domain_right, left, right, false, T(), T());
    }

public:
    Segment_tree() = default;

    explicit Segment_tree(const std::vector<T>& values) {
        init(values);
    }

    void init(const std::vector<T>& values) {
#ifdef ZZ_LIBRARY_DEBUG
        if (values.size() > static_cast<std::size_t>(domain_right + 1)) {
            throw std::length_error("zz_library::Segment_tree< T, lazy >: initial vector exceeds domain");
        }
#endif
        std::vector<Node>().swap(nodes);
        nodes.emplace_back();
        std::vector<int>().swap(free_nodes);
        root = values.empty() ? 0 : build(values, domain_left, domain_right);
    }

    void set(int index, const T& value) {
#ifdef ZZ_LIBRARY_DEBUG
        check_index(index);
#endif
        root = update_assign(root, domain_left, domain_right, index, index, value);
    }

    void add(int index, const T& value) {
#ifdef ZZ_LIBRARY_DEBUG
        check_index(index);
#endif
        root = update_add(root, domain_left, domain_right, index, index, value);
    }

    void update(int left, int right, const T& value) {
#ifdef ZZ_LIBRARY_DEBUG
        check_range(left, right);
#endif
        root = update_add(root, domain_left, domain_right, left, right, value);
    }

    void assign(int left, int right, const T& value) {
#ifdef ZZ_LIBRARY_DEBUG
        check_range(left, right);
#endif
        root = update_assign(root, domain_left, domain_right, left, right, value);
    }

    T query_min(int left, int right) const {
        return query_info(left, right).minimum;
    }

    T query_max(int left, int right) const {
        return query_info(left, right).maximum;
    }

    int where_min(int left, int right) const {
        return query_info(left, right).min_index;
    }

    int where_max(int left, int right) const {
        return query_info(left, right).max_index;
    }

    T query_sum(int left, int right) const {
        return query_info(left, right).sum;
    }

    T query_gcd(int left, int right) const {
        const Info info = query_info(left, right);
        return gcd(absolute(info.first), info.diff_gcd);
    }

    int size() const {
        return domain_right - domain_left + 1;
    }

    bool empty() const {
        return false;
    }
};

} // namespace zz_library

#endif // ZZ_LIBRARY_SEG_TREE_LAZY_HPP
