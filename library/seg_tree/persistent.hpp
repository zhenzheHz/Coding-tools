#ifndef ZZ_LIBRARY_SEG_TREE_PERSISTENT_HPP
#define ZZ_LIBRARY_SEG_TREE_PERSISTENT_HPP

#include "core.hpp"

namespace zz_library {

template <class T>
class Segment_tree<T, persistent> {
private:
    struct Node {
        int left_child = 0;
        int right_child = 0;
        T sum{};
        T minimum{};
        T maximum{};
        T add_tag{};
        T assign_tag{};
        bool has_assignment = false;
    };

    struct Tag {
        T add{};
        T assign{};
        bool has_assignment = false;
    };

    struct Info {
        T sum{};
        T minimum{};
        T maximum{};
        bool valid = false;
    };

    int n = 0;
    std::vector<Node> nodes;
    std::vector<int> roots;

#ifdef ZZ_LIBRARY_DEBUG
    void check_version(int version) const {
        if (version < 0 || static_cast<std::size_t>(version) >= roots.size()) {
            throw std::out_of_range("zz_library::Segment_tree< T, persistent >: version out of range");
        }
    }

    void check_range(int left, int right) const {
        if (left < 0 || right >= n || left > right) {
            throw std::out_of_range("zz_library::Segment_tree< T, persistent >: invalid range");
        }
    }

    void check_index(int index) const {
        if (index < 0 || index >= n) {
            throw std::out_of_range("zz_library::Segment_tree< T, persistent >: index out of range");
        }
    }
#endif

    int create_node(const Node& node = Node{}) {
        nodes.push_back(node);
        return static_cast<int>(nodes.size()) - 1;
    }

    int build(int left, int right, const std::vector<T>& values) {
        const int node = create_node();
        if (left == right) {
            Node& current = nodes[static_cast<std::size_t>(node)];
            current.sum = values[static_cast<std::size_t>(left)];
            current.minimum = values[static_cast<std::size_t>(left)];
            current.maximum = values[static_cast<std::size_t>(left)];
            return node;
        }
        const int middle = left + (right - left) / 2;
        const int left_child = build(left, middle, values);
        const int right_child = build(middle + 1, right, values);
        Node& current = nodes[static_cast<std::size_t>(node)];
        current.left_child = left_child;
        current.right_child = right_child;
        pull(node);
        return node;
    }

    void pull(int node) {
        Node& current = nodes[static_cast<std::size_t>(node)];
        const Node& left = nodes[static_cast<std::size_t>(current.left_child)];
        const Node& right = nodes[static_cast<std::size_t>(current.right_child)];
        current.sum = left.sum + right.sum;
        current.minimum = std::min(left.minimum, right.minimum);
        current.maximum = std::max(left.maximum, right.maximum);
    }

    int clone_node(int node) {
        const Node copy = nodes[static_cast<std::size_t>(node)];
        return create_node(copy);
    }

    void apply_assign(int node, int length, const T& value) {
        Node& current = nodes[static_cast<std::size_t>(node)];
        current.sum = value * T(length);
        current.minimum = value;
        current.maximum = value;
        current.assign_tag = value;
        current.add_tag = T();
        current.has_assignment = true;
    }

    void apply_add(int node, int length, const T& value) {
        Node& current = nodes[static_cast<std::size_t>(node)];
        current.sum += value * T(length);
        current.minimum += value;
        current.maximum += value;
        if (current.has_assignment) current.assign_tag += value;
        else current.add_tag += value;
    }

    void push(int node, int left, int right) {
        const Node& current = nodes[static_cast<std::size_t>(node)];
        if (left == right || (!current.has_assignment && current.add_tag == T())) return;

        const int middle = left + (right - left) / 2;
        const int left_length = middle - left + 1;
        const int right_length = right - middle;
        const int old_left_child = current.left_child;
        const int old_right_child = current.right_child;
        const bool has_assignment = current.has_assignment;
        const T assign_value = current.assign_tag;
        const T add_value = current.add_tag;

        const int left_child = clone_node(old_left_child);
        const int right_child = clone_node(old_right_child);
        nodes[static_cast<std::size_t>(node)].left_child = left_child;
        nodes[static_cast<std::size_t>(node)].right_child = right_child;
        if (has_assignment) {
            apply_assign(left_child, left_length, assign_value);
            apply_assign(right_child, right_length, assign_value);
        } else {
            apply_add(left_child, left_length, add_value);
            apply_add(right_child, right_length, add_value);
        }
        nodes[static_cast<std::size_t>(node)].has_assignment = false;
        nodes[static_cast<std::size_t>(node)].assign_tag = T();
        nodes[static_cast<std::size_t>(node)].add_tag = T();
    }

    void update_range(int node, int left, int right, int query_left, int query_right, const T& value, bool is_assignment) {
        if (query_left <= left && right <= query_right) {
            if (is_assignment) apply_assign(node, right - left + 1, value);
            else apply_add(node, right - left + 1, value);
            return;
        }

        push(node, left, right);
        const int middle = left + (right - left) / 2;
        int left_child = nodes[static_cast<std::size_t>(node)].left_child;
        int right_child = nodes[static_cast<std::size_t>(node)].right_child;
        if (query_left <= middle) {
            left_child = clone_node(left_child);
            nodes[static_cast<std::size_t>(node)].left_child = left_child;
            update_range(left_child, left, middle, query_left, query_right,
                         value, is_assignment);
        }
        if (query_right > middle) {
            right_child = clone_node(right_child);
            nodes[static_cast<std::size_t>(node)].right_child = right_child;
            update_range(right_child, middle + 1, right, query_left, query_right,
                         value, is_assignment);
        }
        pull(node);
    }

    static Tag compose(const Tag& inner, const Tag& outer) {
        if (outer.has_assignment) return outer;
        Tag result = inner;
        if (result.has_assignment) result.assign += outer.add;
        else result.add += outer.add;
        return result;
    }

    static Info apply_tag(Info info, int length, const Tag& tag) {
        if (!info.valid) return info;
        if (tag.has_assignment) {
            info.sum = tag.assign * T(length);
            info.minimum = tag.assign;
            info.maximum = tag.assign;
        }
        if (tag.add != T()) {
            info.sum += tag.add * T(length);
            info.minimum += tag.add;
            info.maximum += tag.add;
        }
        return info;
    }

    static Info merge_info(const Info& left, const Info& right) {
        if (!left.valid) return right;
        if (!right.valid) return left;
        return {left.sum + right.sum, std::min(left.minimum, right.minimum), std::max(left.maximum, right.maximum),
                true};
    }

    Info query_range(int node, int left, int right, int query_left, int query_right, const Tag& inherited_tag) const {
        const Node& current = nodes[static_cast<std::size_t>(node)];
        if (query_left <= left && right <= query_right) {
            return apply_tag({current.sum, current.minimum, current.maximum, true}, right - left + 1, inherited_tag);
        }

        Tag local_tag;
        local_tag.add = current.add_tag;
        local_tag.assign = current.assign_tag;
        local_tag.has_assignment = current.has_assignment;
        const Tag child_tag = compose(local_tag, inherited_tag);
        const int middle = left + (right - left) / 2;
        if (query_right <= middle) {
            return query_range(current.left_child, left, middle, query_left, query_right, child_tag);
        }
        if (query_left > middle) {
            return query_range(current.right_child, middle + 1, right, query_left, query_right, child_tag);
        }
        return merge_info(query_range(current.left_child, left, middle, query_left, query_right, child_tag),
                          query_range(current.right_child, middle + 1, right, query_left, query_right, child_tag));
    }

    int update_from(int version, int left, int right, const T& value, bool is_assignment) {
#ifdef ZZ_LIBRARY_DEBUG
        check_version(version);
        check_range(left, right);
#endif
        const int new_root = clone_node(roots[static_cast<std::size_t>(version)]);
        update_range(new_root, 0, n - 1, left, right, value, is_assignment);
        roots.push_back(new_root);
        return static_cast<int>(roots.size()) - 1;
    }

public:
    Segment_tree() {
        nodes.emplace_back();
    }

    explicit Segment_tree(const std::vector<T>& values) {
        init(values);
    }

    void init(const std::vector<T>& values) {
        n = static_cast<int>(values.size());
        nodes.clear();
        nodes.emplace_back();
        roots.clear();
        roots.push_back(n == 0 ? 0 : build(0, n - 1, values));
    }

    int size() const {
        return n;
    }

    bool empty() const {
        return n == 0;
    }

    int version_count() const {
        return static_cast<int>(roots.size());
    }

    int latest_version() const {
        return static_cast<int>(roots.size()) - 1;
    }

    int update(int left, int right, const T& value) {
        return update_from(latest_version(), left, right, value, false);
    }

    int update(int version, int left, int right, const T& value) {
        return update_from(version, left, right, value, false);
    }

    int assign(int left, int right, const T& value) {
        return update_from(latest_version(), left, right, value, true);
    }

    int assign(int version, int left, int right, const T& value) {
        return update_from(version, left, right, value, true);
    }

    int add(int index, const T& value) {
        return update_from(latest_version(), index, index, value, false);
    }

    int add(int version, int index, const T& value) {
        return update_from(version, index, index, value, false);
    }

    int set(int index, const T& value) {
        return update_from(latest_version(), index, index, value, true);
    }

    int set(int version, int index, const T& value) {
        return update_from(version, index, index, value, true);
    }

    T query_sum(int left, int right) const {
        return query_sum(latest_version(), left, right);
    }

    T query_sum(int version, int left, int right) const {
#ifdef ZZ_LIBRARY_DEBUG
        check_version(version);
        check_range(left, right);
#endif
        return query_range(roots[static_cast<std::size_t>(version)], 0, n - 1, left, right, Tag{}).sum;
    }

    T query_min(int left, int right) const {
        return query_min(latest_version(), left, right);
    }

    T query_min(int version, int left, int right) const {
#ifdef ZZ_LIBRARY_DEBUG
        check_version(version);
        check_range(left, right);
#endif
        return query_range(roots[static_cast<std::size_t>(version)], 0, n - 1, left, right, Tag{}).minimum;
    }

    T query_max(int left, int right) const {
        return query_max(latest_version(), left, right);
    }

    T query_max(int version, int left, int right) const {
#ifdef ZZ_LIBRARY_DEBUG
        check_version(version);
        check_range(left, right);
#endif
        return query_range(roots[static_cast<std::size_t>(version)], 0, n - 1, left, right, Tag{}).maximum;
    }
};

} // namespace zz_library

#endif // ZZ_LIBRARY_SEG_TREE_PERSISTENT_HPP
