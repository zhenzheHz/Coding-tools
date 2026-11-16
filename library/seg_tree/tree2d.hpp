#ifndef ZZ_LIBRARY_SEG_TREE_TREE2D_HPP
#define ZZ_LIBRARY_SEG_TREE_TREE2D_HPP

#include "core.hpp"

namespace zz_library {

template <class T>
class Segment_tree<T, tree2d> {
private:
    // Each outer x-node owns a randomized y-treap. A y-node stores the
    // aggregate for that y-coordinate and the subtree aggregate over y.
    struct Inner_node {
        std::int64_t y = 0;
        T point_sum{};
        T point_max{};
        T sum{};
        T maximum{};
        int left = 0;
        int right = 0;
        int size = 1;
        std::uint32_t priority = 0;
        // The int-indexed node pool bounds the number of live points below INT_MAX.
        std::uint32_t point_mark = 0;
        std::uint32_t mark = 0;
    };

    struct Outer_node {
        int left = 0;
        int right = 0;
        int inner_root = 0;
    };

    struct Info {
        T sum{};
        T maximum{};
        std::uint64_t mark = 0;
    };

    std::vector<Inner_node> inner_nodes{Inner_node{}};
    std::vector<Outer_node> outer_nodes{Outer_node{}};
    std::vector<int> free_inner_nodes;
    std::vector<int> free_outer_nodes;
    std::mt19937_64 inner_rng{std::random_device{}()};

    int root = 0;
    std::int64_t x_left = 0;
    std::int64_t x_right = -1;
    std::int64_t y_left = 0;
    std::int64_t y_right = -1;
    bool has_x_domain = false;
    bool has_y_domain = false;

    static std::int64_t midpoint(std::int64_t left, std::int64_t right) {
        const std::uint64_t distance = static_cast<std::uint64_t>(right) - static_cast<std::uint64_t>(left);
        return left + static_cast<std::int64_t>(distance / 2);
    }

#ifdef ZZ_LIBRARY_DEBUG
    void check_domains() const {
        if (!has_x_domain || !has_y_domain) {
            throw std::logic_error("zz_library::Segment_tree< T, tree2d >: set both domains first");
        }
    }

    void check_point(std::int64_t x, std::int64_t y) const {
        check_domains();
        if (x < x_left || x > x_right || y < y_left || y > y_right) {
            throw std::out_of_range("zz_library::Segment_tree< T, tree2d >: point out of range");
        }
    }

    void check_rectangle(std::int64_t x1, std::int64_t x2, std::int64_t y1, std::int64_t y2) const {
        check_domains();
        if (x1 > x2 || y1 > y2) {
            throw std::invalid_argument("zz_library::Segment_tree< T, tree2d >: invalid rectangle order");
        }
        if (x1 < x_left || x2 > x_right || y1 < y_left || y2 > y_right) {
            throw std::out_of_range("zz_library::Segment_tree< T, tree2d >: rectangle out of range");
        }
    }
#endif

    void reset_storage() {
        std::vector<Inner_node>().swap(inner_nodes);
        inner_nodes.emplace_back();
        std::vector<Outer_node>().swap(outer_nodes);
        outer_nodes.emplace_back();
        std::vector<int>().swap(free_inner_nodes);
        std::vector<int>().swap(free_outer_nodes);
        root = 0;
    }

    int allocate_inner_node() {
        if (free_inner_nodes.empty()) {
            inner_nodes.emplace_back();
            return static_cast<int>(inner_nodes.size() - 1);
        }
        const int index = free_inner_nodes.back();
        free_inner_nodes.pop_back();
        inner_nodes[static_cast<std::size_t>(index)] = Inner_node{};
        return index;
    }

    int allocate_outer_node() {
        if (free_outer_nodes.empty()) {
            outer_nodes.emplace_back();
            return static_cast<int>(outer_nodes.size() - 1);
        }
        const int index = free_outer_nodes.back();
        free_outer_nodes.pop_back();
        outer_nodes[static_cast<std::size_t>(index)] = Outer_node{};
        return index;
    }

    void release_inner_node(int index) {
        inner_nodes[static_cast<std::size_t>(index)] = Inner_node{};
        free_inner_nodes.push_back(index);
    }

    void release_inner_subtree(int index) {
        if (!index) return;
        const int left = inner_nodes[static_cast<std::size_t>(index)].left;
        const int right = inner_nodes[static_cast<std::size_t>(index)].right;
        release_inner_subtree(left);
        release_inner_subtree(right);
        release_inner_node(index);
    }

    void release_outer_node(int index) {
        outer_nodes[static_cast<std::size_t>(index)] = Outer_node{};
        free_outer_nodes.push_back(index);
    }

    static Info merge(const Info& left, const Info& right) {
        Info result;
        result.sum = left.sum + right.sum;
        result.mark = left.mark + right.mark;
        if (!left.mark) result.maximum = right.maximum;
        else if (!right.mark) result.maximum = left.maximum;
        else result.maximum = std::max(left.maximum, right.maximum);
        return result;
    }

    void pull_inner(int index) {
        Inner_node& node = inner_nodes[static_cast<std::size_t>(index)];
        const Inner_node* left = node.left ? &inner_nodes[static_cast<std::size_t>(node.left)] : nullptr;
        const Inner_node* right = node.right ? &inner_nodes[static_cast<std::size_t>(node.right)] : nullptr;
        node.size = 1 + (left ? left->size : 0) + (right ? right->size : 0);
        node.sum = node.point_sum + (left ? left->sum : T()) + (right ? right->sum : T());
        node.mark = node.point_mark + (left ? left->mark : 0U) + (right ? right->mark : 0U);
        bool has_value = node.point_mark != 0;
        node.maximum = has_value ? node.point_max : T();
        if (left && left->mark) {
            node.maximum = has_value ? std::max(node.maximum, left->maximum) : left->maximum;
            has_value = true;
        }
        if (right && right->mark) {
            node.maximum = has_value ? std::max(node.maximum, right->maximum) : right->maximum;
            has_value = true;
        }
        if (!has_value) node.maximum = T();
    }

    int rotate_inner_right(int index) {
        const int top = inner_nodes[static_cast<std::size_t>(index)].left;
        inner_nodes[static_cast<std::size_t>(index)].left = inner_nodes[static_cast<std::size_t>(top)].right;
        inner_nodes[static_cast<std::size_t>(top)].right = index;
        pull_inner(index);
        pull_inner(top);
        return top;
    }

    int rotate_inner_left(int index) {
        const int top = inner_nodes[static_cast<std::size_t>(index)].right;
        inner_nodes[static_cast<std::size_t>(index)].right = inner_nodes[static_cast<std::size_t>(top)].left;
        inner_nodes[static_cast<std::size_t>(top)].left = index;
        pull_inner(index);
        pull_inner(top);
        return top;
    }

    int merge_inner(int left, int right) {
        if (!left) return right;
        if (!right) return left;
        if (inner_nodes[static_cast<std::size_t>(left)].priority > inner_nodes[static_cast<std::size_t>(right)].priority) {
            const int child = merge_inner(inner_nodes[static_cast<std::size_t>(left)].right, right);
            inner_nodes[static_cast<std::size_t>(left)].right = child;
            pull_inner(left);
            return left;
        }
        const int child = merge_inner(left, inner_nodes[static_cast<std::size_t>(right)].left);
        inner_nodes[static_cast<std::size_t>(right)].left = child;
        pull_inner(right);
        return right;
    }

    Info inner_point(int index, std::int64_t y) const {
        while (index) {
            const Inner_node& node = inner_nodes[static_cast<std::size_t>(index)];
            if (y == node.y) return {node.point_sum, node.point_max, node.point_mark};
            index = y < node.y ? node.left : node.right;
        }
        return {};
    }

    int update_inner(int index, std::int64_t y, const Info& value) {
        if (!index) {
            if (!value.mark) return 0;
            index = allocate_inner_node();
            Inner_node& node = inner_nodes[static_cast<std::size_t>(index)];
            node.y = y;
            node.priority = static_cast<std::uint32_t>(inner_rng() >> 32);
            node.point_sum = node.sum = value.sum;
            node.point_max = node.maximum = value.maximum;
            node.point_mark = node.mark = static_cast<std::uint32_t>(value.mark);
            return index;
        }

        const std::int64_t key = inner_nodes[static_cast<std::size_t>(index)].y;
        if (y == key) {
            if (!value.mark) {
                const int left = inner_nodes[static_cast<std::size_t>(index)].left;
                const int right = inner_nodes[static_cast<std::size_t>(index)].right;
                const int replacement = merge_inner(left, right);
                release_inner_node(index);
                return replacement;
            }
            Inner_node& node = inner_nodes[static_cast<std::size_t>(index)];
            node.point_sum = value.sum;
            node.point_max = value.maximum;
            node.point_mark = static_cast<std::uint32_t>(value.mark);
            pull_inner(index);
            return index;
        }

        if (y < key) {
            const int child = update_inner(inner_nodes[static_cast<std::size_t>(index)].left, y, value);
            inner_nodes[static_cast<std::size_t>(index)].left = child;
            if (child && inner_nodes[static_cast<std::size_t>(child)].priority > inner_nodes[static_cast<std::size_t>(index)].priority)
                return rotate_inner_right(index);
        } else {
            const int child = update_inner(inner_nodes[static_cast<std::size_t>(index)].right, y, value);
            inner_nodes[static_cast<std::size_t>(index)].right = child;
            if (child && inner_nodes[static_cast<std::size_t>(child)].priority > inner_nodes[static_cast<std::size_t>(index)].priority)
                return rotate_inner_left(index);
        }
        pull_inner(index);
        return index;
    }

    std::uint64_t inner_count_less(int index, std::int64_t y, bool include_equal) const {
        std::uint64_t result = 0;
        while (index) {
            const Inner_node& node = inner_nodes[static_cast<std::size_t>(index)];
            if (node.y < y || (include_equal && node.y == y)) {
                result += static_cast<std::uint64_t>(node.left ? inner_nodes[static_cast<std::size_t>(node.left)].size : 0) + 1;
                index = node.right;
            } else {
                index = node.left;
            }
        }
        return result;
    }

    Info inner_rank_range(int index, std::uint64_t first, std::uint64_t last) const {
        if (!index || first >= last) return {};
        const Inner_node& node = inner_nodes[static_cast<std::size_t>(index)];
        const std::uint64_t size = static_cast<std::uint64_t>(node.size);
        if (first == 0 && last >= size) return {node.sum, node.maximum, node.mark};

        const std::uint64_t left_size = node.left
            ? static_cast<std::uint64_t>(inner_nodes[static_cast<std::size_t>(node.left)].size) : 0;
        Info result;
        if (first < left_size) {
            result = merge(result, inner_rank_range(node.left, first, std::min(last, left_size)));
        }
        if (first <= left_size && left_size < last) {
            result = merge(result, {node.point_sum, node.point_max, node.point_mark});
        }
        const std::uint64_t right_begin = left_size + 1;
        if (last > right_begin) {
            result = merge(result, inner_rank_range(node.right,
                first > right_begin ? first - right_begin : 0, last - right_begin));
        }
        return result;
    }

    Info query_inner(int index, std::int64_t query_left, std::int64_t query_right) const {
        if (!index || query_left > query_right) return {};
        const std::uint64_t first = inner_count_less(index, query_left, false);
        const std::uint64_t last = inner_count_less(index, query_right, true);
        return inner_rank_range(index, first, last);
    }

    int update_outer(int index, std::int64_t left, std::int64_t right,
                     std::int64_t x, std::int64_t y, const T& value, bool assignment) {
        if (!index && value == T()) return 0;
        if (!index) index = allocate_outer_node();

        if (left == right) {
            const int old_inner = outer_nodes[static_cast<std::size_t>(index)].inner_root;
            const Info old_value = inner_point(old_inner, y);
            const T new_value = assignment ? value : old_value.sum + value;
            const Info point_value{new_value, new_value, new_value == T() ? 0U : 1U};
            const int new_inner = update_inner(old_inner, y, point_value);
            outer_nodes[static_cast<std::size_t>(index)].inner_root = new_inner;
            if (!new_inner) {
                release_outer_node(index);
                return 0;
            }
            return index;
        }

        const std::int64_t middle = midpoint(left, right);
        if (x <= middle) {
            const int child = update_outer(outer_nodes[static_cast<std::size_t>(index)].left,
                left, middle, x, y, value, assignment);
            outer_nodes[static_cast<std::size_t>(index)].left = child;
        } else {
            const int child = update_outer(outer_nodes[static_cast<std::size_t>(index)].right,
                middle + 1, right, x, y, value, assignment);
            outer_nodes[static_cast<std::size_t>(index)].right = child;
        }

        const int child_left = outer_nodes[static_cast<std::size_t>(index)].left;
        const int child_right = outer_nodes[static_cast<std::size_t>(index)].right;
        const int inner_left = child_left ? outer_nodes[static_cast<std::size_t>(child_left)].inner_root : 0;
        const int inner_right = child_right ? outer_nodes[static_cast<std::size_t>(child_right)].inner_root : 0;
        const Info combined = merge(inner_point(inner_left, y), inner_point(inner_right, y));
        const int old_inner = outer_nodes[static_cast<std::size_t>(index)].inner_root;
        const int new_inner = update_inner(old_inner, y, combined);
        outer_nodes[static_cast<std::size_t>(index)].inner_root = new_inner;

        if (!child_left && !child_right) {
            if (new_inner) release_inner_subtree(new_inner);
            release_outer_node(index);
            return 0;
        }
        return index;
    }

    Info query_outer(int index, std::int64_t left, std::int64_t right,
                     std::int64_t query_x1, std::int64_t query_x2,
                     std::int64_t query_y1, std::int64_t query_y2) const {
        if (query_x2 < left || right < query_x1 || !index) return {};
        if (query_x1 <= left && right <= query_x2) {
            return query_inner(outer_nodes[static_cast<std::size_t>(index)].inner_root, query_y1, query_y2);
        }
        const std::int64_t middle = midpoint(left, right);
        const Outer_node& node = outer_nodes[static_cast<std::size_t>(index)];
        return merge(query_outer(node.left, left, middle, query_x1, query_x2, query_y1, query_y2),
                     query_outer(node.right, middle + 1, right, query_x1, query_x2, query_y1, query_y2));
    }

    Info query_rectangle(std::int64_t x1, std::int64_t x2,
                         std::int64_t y1, std::int64_t y2) const {
#ifdef ZZ_LIBRARY_DEBUG
        check_rectangle(x1, x2, y1, y2);
#endif
        return query_outer(root, x_left, x_right, x1, x2, y1, y2);
    }

    static bool rectangle_has_zero_cell(std::int64_t x1, std::int64_t x2,
                                        std::int64_t y1, std::int64_t y2,
                                        std::uint64_t nonzero_points) {
        using wide = __uint128_t;
        const wide width = static_cast<wide>(static_cast<std::uint64_t>(x2) - static_cast<std::uint64_t>(x1)) + 1;
        const wide height = static_cast<wide>(static_cast<std::uint64_t>(y2) - static_cast<std::uint64_t>(y1)) + 1;
        if (width > nonzero_points || height > nonzero_points) return true;
        return width * height > nonzero_points;
    }

public:
    Segment_tree() = default;

    void set_domain(std::int64_t new_x_left, std::int64_t new_x_right,
                    std::int64_t new_y_left, std::int64_t new_y_right) {
#ifdef ZZ_LIBRARY_DEBUG
        if (new_x_left > new_x_right || new_y_left > new_y_right) {
            throw std::invalid_argument("zz_library::Segment_tree< T, tree2d >: invalid domain");
        }
#endif
        reset_storage();
        x_left = new_x_left;
        x_right = new_x_right;
        y_left = new_y_left;
        y_right = new_y_right;
        has_x_domain = true;
        has_y_domain = true;
    }

    void set_x_domain(std::int64_t left, std::int64_t right) {
#ifdef ZZ_LIBRARY_DEBUG
        if (left > right) {
            throw std::invalid_argument("zz_library::Segment_tree< T, tree2d >: invalid x domain");
        }
#endif
        reset_storage();
        x_left = left;
        x_right = right;
        has_x_domain = true;
    }

    void set_y_domain(std::int64_t left, std::int64_t right) {
#ifdef ZZ_LIBRARY_DEBUG
        if (left > right) {
            throw std::invalid_argument("zz_library::Segment_tree< T, tree2d >: invalid y domain");
        }
#endif
        reset_storage();
        y_left = left;
        y_right = right;
        has_y_domain = true;
    }

    void assign(std::int64_t x, std::int64_t y, const T& value) {
#ifdef ZZ_LIBRARY_DEBUG
        check_point(x, y);
#endif
        root = update_outer(root, x_left, x_right, x, y, value, true);
    }

    void add(std::int64_t x, std::int64_t y, const T& value) {
#ifdef ZZ_LIBRARY_DEBUG
        check_point(x, y);
#endif
        if (value == T()) return;
        root = update_outer(root, x_left, x_right, x, y, value, false);
    }

    T query_sum(std::int64_t x1, std::int64_t x2,
                std::int64_t y1, std::int64_t y2) const {
        return query_rectangle(x1, x2, y1, y2).sum;
    }

    std::uint64_t query_mark(std::int64_t x1, std::int64_t x2,
                             std::int64_t y1, std::int64_t y2) const {
        return query_rectangle(x1, x2, y1, y2).mark;
    }

    T query_max(std::int64_t x1, std::int64_t x2,
                std::int64_t y1, std::int64_t y2) const {
        const Info result = query_rectangle(x1, x2, y1, y2);
        return rectangle_has_zero_cell(x1, x2, y1, y2, result.mark)
            ? std::max(result.maximum, T()) : result.maximum;
    }
};

} // namespace zz_library

#endif // ZZ_LIBRARY_SEG_TREE_TREE2D_HPP
