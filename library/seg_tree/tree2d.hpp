#ifndef ZZ_LIBRARY_SEG_TREE_TREE2D_HPP
#define ZZ_LIBRARY_SEG_TREE_TREE2D_HPP

#include "core.hpp"

namespace zz_library {

template <class T>
class Segment_tree<T, tree2d> {
private:
    // Each outer node owns a y-tree aggregating all x-coordinates in its interval.
    struct Inner_node {
        T sum{};
        T maximum{};
        std::uint64_t mark = 0;
        int left = 0;
        int right = 0;
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
        bool valid = false;
    };

    std::vector<Inner_node> inner_nodes{Inner_node{}};
    std::vector<Outer_node> outer_nodes{Outer_node{}};
    std::vector<int> free_inner_nodes;
    std::vector<int> free_outer_nodes;

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

    void release_inner_subtree(int index) {
        if (!index) return;
        const int left = inner_nodes[static_cast<std::size_t>(index)].left;
        const int right = inner_nodes[static_cast<std::size_t>(index)].right;
        release_inner_subtree(left);
        release_inner_subtree(right);
        inner_nodes[static_cast<std::size_t>(index)] = Inner_node{};
        free_inner_nodes.push_back(index);
    }

    void release_outer_node(int index) {
        if (!index) return;
        outer_nodes[static_cast<std::size_t>(index)] = Outer_node{};
        free_outer_nodes.push_back(index);
    }

    static Info merge(const Info& left, const Info& right) {
        if (!left.valid) return right;
        if (!right.valid) return left;
        return {left.sum + right.sum, std::max(left.maximum, right.maximum), left.mark + right.mark, true};
    }

    Info inner_point(int index, std::int64_t left, std::int64_t right, std::int64_t position) const {
        while (index && left < right) {
            const std::int64_t middle = midpoint(left, right);
            if (position <= middle) {
                index = inner_nodes[static_cast<std::size_t>(index)].left;
                right = middle;
            } else {
                index = inner_nodes[static_cast<std::size_t>(index)].right;
                left = middle + 1;
            }
        }
        if (!index) return {T(), T(), 0, true};
        const Inner_node& node = inner_nodes[static_cast<std::size_t>(index)];
        return {node.sum, node.maximum, node.mark, true};
    }

    int update_inner(int index, std::int64_t left, std::int64_t right, std::int64_t position, const Info& value) {
        if (!index && !value.mark) return 0;
        if (left == right) {
            if (!value.mark) {
                release_inner_subtree(index);
                return 0;
            }
            if (!index) index = allocate_inner_node();
            Inner_node& node = inner_nodes[static_cast<std::size_t>(index)];
            node.sum = value.sum;
            node.maximum = value.maximum;
            node.mark = value.mark;
            return index;
        }

        if (!index) index = allocate_inner_node();
        const std::int64_t middle = midpoint(left, right);
        if (position <= middle) {
            const int child = update_inner(inner_nodes[static_cast<std::size_t>(index)].left, left, middle, position, value);
            inner_nodes[static_cast<std::size_t>(index)].left = child;
        } else {
            const int child = update_inner(inner_nodes[static_cast<std::size_t>(index)].right, middle + 1, right, position, value);
            inner_nodes[static_cast<std::size_t>(index)].right = child;
        }

        const int child_left = inner_nodes[static_cast<std::size_t>(index)].left;
        const int child_right = inner_nodes[static_cast<std::size_t>(index)].right;
        if (!child_left && !child_right) {
            release_inner_subtree(index);
            return 0;
        }
        const Inner_node& lhs = inner_nodes[static_cast<std::size_t>(child_left)];
        const Inner_node& rhs = inner_nodes[static_cast<std::size_t>(child_right)];
        Inner_node& node = inner_nodes[static_cast<std::size_t>(index)];
        node.sum = lhs.sum + rhs.sum;
        node.maximum = std::max(child_left ? lhs.maximum : T(), child_right ? rhs.maximum : T());
        node.mark = lhs.mark + rhs.mark;
        return index;
    }

    int update_outer(int index, std::int64_t left, std::int64_t right, std::int64_t x, std::int64_t y, const T& value, bool assignment) {
        if (!index && value == T()) return 0;
        if (!index) index = allocate_outer_node();

        if (left == right) {
            const int old_inner = outer_nodes[static_cast<std::size_t>(index)].inner_root;
            const Info old_value = inner_point(old_inner, y_left, y_right, y);
            const T new_value = assignment ? value : old_value.sum + value;
            Info point_value{new_value, new_value, new_value == T() ? 0U : 1U, true};
            const int new_inner = update_inner(old_inner, y_left, y_right, y, point_value);
            outer_nodes[static_cast<std::size_t>(index)].inner_root = new_inner;
            if (!new_inner) {
                release_outer_node(index);
                return 0;
            }
            return index;
        }

        const std::int64_t middle = midpoint(left, right);
        if (x <= middle) {
            const int child = update_outer(outer_nodes[static_cast<std::size_t>(index)].left, left, middle, x, y, value, assignment);
            outer_nodes[static_cast<std::size_t>(index)].left = child;
        } else {
            const int child = update_outer(outer_nodes[static_cast<std::size_t>(index)].right, middle + 1, right, x, y, value, assignment);
            outer_nodes[static_cast<std::size_t>(index)].right = child;
        }

        const int child_left = outer_nodes[static_cast<std::size_t>(index)].left;
        const int child_right = outer_nodes[static_cast<std::size_t>(index)].right;
        const int inner_left = child_left? outer_nodes[static_cast<std::size_t>(child_left)].inner_root : 0;
        const int inner_right = child_right? outer_nodes[static_cast<std::size_t>(child_right)].inner_root : 0;
        const Info lhs = inner_point(inner_left, y_left, y_right, y);
        const Info rhs = inner_point(inner_right, y_left, y_right, y);
        const Info combined = merge(lhs, rhs);
        const int old_inner = outer_nodes[static_cast<std::size_t>(index)].inner_root;
        const int new_inner = update_inner(old_inner, y_left, y_right, y, combined);
        outer_nodes[static_cast<std::size_t>(index)].inner_root = new_inner;

        if (!child_left && !child_right) {
            release_outer_node(index);
            return 0;
        }
        return index;
    }

    Info query_inner(int index, std::int64_t left, std::int64_t right, std::int64_t query_left, std::int64_t query_right) const {
        if (query_right < left || right < query_left) return {};
        if (!index) return {T(), T(), 0, true};
        if (query_left <= left && right <= query_right) {
            const Inner_node& node = inner_nodes[static_cast<std::size_t>(index)];
            return {node.sum, node.maximum, node.mark, true};
        }
        const std::int64_t middle = midpoint(left, right);
        const Inner_node& node = inner_nodes[static_cast<std::size_t>(index)];
        return merge(query_inner(node.left, left, middle, query_left, query_right), query_inner(node.right, middle + 1, right, query_left, query_right));
    }

    Info query_outer(int index, std::int64_t left, std::int64_t right, std::int64_t query_x1, std::int64_t query_x2, std::int64_t query_y1, std::int64_t query_y2) const {
        if (query_x2 < left || right < query_x1) return {};
        if (!index) return {T(), T(), 0, true};
        if (query_x1 <= left && right <= query_x2) {
            const int inner_root = outer_nodes[static_cast<std::size_t>(index)].inner_root;
            return query_inner(inner_root, y_left, y_right, query_y1, query_y2);
        }
        const std::int64_t middle = midpoint(left, right);
        const Outer_node& node = outer_nodes[static_cast<std::size_t>(index)];
        return merge(query_outer(node.left, left, middle, query_x1, query_x2, query_y1, query_y2), query_outer(node.right, middle + 1, right, query_x1, query_x2, query_y1, query_y2));
    }

    Info query_rectangle(std::int64_t x1, std::int64_t x2, std::int64_t y1, std::int64_t y2) const {
#ifdef ZZ_LIBRARY_DEBUG
        check_rectangle(x1, x2, y1, y2);
#endif
        return query_outer(root, x_left, x_right, x1, x2, y1, y2);
    }

public:
    Segment_tree() = default;

    void set_domain(std::int64_t new_x_left, std::int64_t new_x_right, std::int64_t new_y_left, std::int64_t new_y_right) {
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

    T query_sum(std::int64_t x1, std::int64_t x2, std::int64_t y1, std::int64_t y2) const {
        return query_rectangle(x1, x2, y1, y2).sum;
    }

    std::uint64_t query_mark(std::int64_t x1, std::int64_t x2, std::int64_t y1, std::int64_t y2) const {
        return query_rectangle(x1, x2, y1, y2).mark;
    }

    T query_max(std::int64_t x1, std::int64_t x2, std::int64_t y1, std::int64_t y2) const {
        return query_rectangle(x1, x2, y1, y2).maximum;
    }
};

} // namespace zz_library

#endif // ZZ_LIBRARY_SEG_TREE_TREE2D_HPP
