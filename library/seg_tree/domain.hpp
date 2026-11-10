#ifndef ZZ_LIBRARY_SEG_TREE_DOMAIN_HPP
#define ZZ_LIBRARY_SEG_TREE_DOMAIN_HPP

#include "core.hpp"

namespace zz_library {

template <class T>
class Segment_tree<T, domain> {
private:
    struct Node {
        T sum{};
        T minimum{};
        T maximum{};
        std::uint64_t mark = 0;
        // Number of positions attaining minimum; used to update mark after lazy adds.
        std::uint64_t minimum_count = 0;
        T add_tag{};
        T assign_tag{};
        int left = 0;
        int right = 0;
        bool has_assignment = false;
    };

    struct Info {
        T sum{};
        T minimum{};
        T maximum{};
        std::uint64_t mark = 0;
        std::uint64_t minimum_count = 0;
        bool valid = false;
    };

    struct Piece {
        int node = 0;
        std::int64_t left = 0;
        std::int64_t right = 0;
        bool carry_assignment = false;
        T carry_assign_value{};
        T carry_add{};
    };

    static constexpr int max_cover_pieces = 130;

    std::vector<Node> nodes{Node{}};
    std::vector<int> free_nodes;
    int root = 0;
    std::int64_t domain_left = 0;
    std::int64_t domain_right = -1;
    bool initialized = false;

    static std::int64_t midpoint(std::int64_t left, std::int64_t right) {
        const std::uint64_t distance = static_cast<std::uint64_t>(right)
                                     - static_cast<std::uint64_t>(left);
        return left + static_cast<std::int64_t>(distance / 2);
    }

    static std::uint64_t segment_length(std::int64_t left, std::int64_t right) {
        return static_cast<std::uint64_t>(right) - static_cast<std::uint64_t>(left) + 1;
    }

    static T sum_for(const T& value, std::int64_t left, std::int64_t right) {
        return value * static_cast<T>(segment_length(left, right));
    }

    static Info zero_info(std::int64_t left, std::int64_t right) {
        return {T(), T(), T(), 0, segment_length(left, right), true};
    }

#ifdef ZZ_LIBRARY_DEBUG
    void check_initialized() const {
        if (!initialized) {
            throw std::logic_error("zz_library::Segment_tree< T, domain >: call set_domain first");
        }
    }

    void check_range(std::int64_t left, std::int64_t right) const {
        check_initialized();
        if (left > right || left < domain_left || right > domain_right) {
            throw std::out_of_range("zz_library::Segment_tree< T, domain >: invalid range");
        }
    }

    void check_position(std::int64_t position) const {
        check_initialized();
        if (position < domain_left || position > domain_right) {
            throw std::out_of_range("zz_library::Segment_tree< T, domain >: index out of range");
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

    void apply_assign(int index, std::int64_t left, std::int64_t right, const T& value) {
        const int old_left = nodes[static_cast<std::size_t>(index)].left;
        const int old_right = nodes[static_cast<std::size_t>(index)].right;
        release_subtree(old_left);
        release_subtree(old_right);

        Node& node = nodes[static_cast<std::size_t>(index)];
        const std::uint64_t length = segment_length(left, right);
        node.sum = sum_for(value, left, right);
        node.minimum = value;
        node.maximum = value;
        node.mark = value == T() ? 0 : length;
        node.minimum_count = length;
        node.add_tag = T();
        node.assign_tag = value;
        node.left = 0;
        node.right = 0;
        node.has_assignment = value != T();
    }

    void apply_add(int index, std::int64_t left, std::int64_t right, const T& value) {
        Node& node = nodes[static_cast<std::size_t>(index)];
        const T old_minimum = node.minimum;
        const std::uint64_t length = segment_length(left, right);
        if (!node.left && !node.right) node.minimum_count = length;
        node.sum += sum_for(value, left, right);
        node.minimum += value;
        node.maximum += value;

        // All weights stay nonnegative: positive adds fill zero positions;
        // negative adds can only turn minimum-valued positions into zero.
        if (value > T()) {
            if (old_minimum == T()) node.mark = length;
        } else if (value < T() && node.minimum == T()) {
            node.mark -= node.minimum_count;
        }

        if (node.has_assignment) {
            node.assign_tag += value;
            if (node.assign_tag == T()) {
                node.has_assignment = false;
                node.assign_tag = T();
            }
        }
        else node.add_tag += value;
    }

    void push(int index, std::int64_t left, std::int64_t right) {
        if (left == right) return;
        int child_left = nodes[static_cast<std::size_t>(index)].left;
        int child_right = nodes[static_cast<std::size_t>(index)].right;
        const std::int64_t middle = midpoint(left, right);

        if (nodes[static_cast<std::size_t>(index)].has_assignment) {
            const T value = nodes[static_cast<std::size_t>(index)].assign_tag;
            if (!child_left) child_left = allocate_node();
            if (!child_right) child_right = allocate_node();
            apply_assign(child_left, left, middle, value);
            apply_assign(child_right, middle + 1, right, value);
            nodes[static_cast<std::size_t>(index)].has_assignment = false;
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
        if (child_left && can_prune(child_left)) {
            release_subtree(child_left);
            nodes[static_cast<std::size_t>(index)].left = 0;
        }
        if (child_right && can_prune(child_right)) {
            release_subtree(child_right);
            nodes[static_cast<std::size_t>(index)].right = 0;
        }
    }

    void pull(int index, std::int64_t left, std::int64_t right) {
        const int child_left = nodes[static_cast<std::size_t>(index)].left;
        const int child_right = nodes[static_cast<std::size_t>(index)].right;
        const Node& lhs = nodes[static_cast<std::size_t>(child_left)];
        const Node& rhs = nodes[static_cast<std::size_t>(child_right)];
        const std::int64_t middle = midpoint(left, right);

        Node& node = nodes[static_cast<std::size_t>(index)];
        node.sum = lhs.sum + rhs.sum;
        node.maximum = std::max(child_left ? lhs.maximum : T(), child_right ? rhs.maximum : T());
        node.mark = lhs.mark + rhs.mark;

        const T left_min = child_left ? lhs.minimum : T();
        const T right_min = child_right ? rhs.minimum : T();
        const std::uint64_t left_min_count = child_left? lhs.minimum_count : segment_length(left, middle);
        const std::uint64_t right_min_count = child_right? rhs.minimum_count : segment_length(middle + 1, right);
        node.minimum = std::min(left_min, right_min);
        node.minimum_count = (left_min == node.minimum ? left_min_count : 0) + (right_min == node.minimum ? right_min_count : 0);
    }

    int update_set(int index, std::int64_t left, std::int64_t right, std::int64_t position, const T& value) {
        if (!index && value == T()) return 0;
        if (left == right) {
            if (value == T()) {
                release_subtree(index);
                return 0;
            }
            if (!index) index = allocate_node();
            Node& node = nodes[static_cast<std::size_t>(index)];
            node.sum = value;
            node.minimum = value;
            node.maximum = value;
            node.mark = 1;
            node.minimum_count = 1;
            node.add_tag = T();
            node.assign_tag = T();
            node.has_assignment = false;
            return index;
        }

        if (!index) index = allocate_node();
        push(index, left, right);
        const std::int64_t middle = midpoint(left, right);
        if (position <= middle) {
            const int child = update_set(nodes[static_cast<std::size_t>(index)].left, left, middle, position, value);
            nodes[static_cast<std::size_t>(index)].left = child;
        } else {
            const int child = update_set(nodes[static_cast<std::size_t>(index)].right, middle + 1, right, position, value);
            nodes[static_cast<std::size_t>(index)].right = child;
        }

        pull(index, left, right);
        if (!nodes[static_cast<std::size_t>(index)].left && !nodes[static_cast<std::size_t>(index)].right) {
            release_subtree(index);
            return 0;
        }
        return index;
    }

    int update_add(int index, std::int64_t left, std::int64_t right, std::int64_t position, const T& value) {
        if (value == T()) return index;
        if (left == right) {
            const T old_value = index ? nodes[static_cast<std::size_t>(index)].sum : T();
            const T new_value = old_value + value;
            if (new_value == T()) {
                release_subtree(index);
                return 0;
            }
            if (!index) index = allocate_node();
            Node& node = nodes[static_cast<std::size_t>(index)];
            node.sum = new_value;
            node.minimum = new_value;
            node.maximum = new_value;
            node.mark = 1;
            node.minimum_count = 1;
            node.add_tag = T();
            node.assign_tag = T();
            node.has_assignment = false;
            return index;
        }

        if (!index) index = allocate_node();
        push(index, left, right);
        const std::int64_t middle = midpoint(left, right);
        if (position <= middle) {
            const int child = update_add(nodes[static_cast<std::size_t>(index)].left, left, middle, position, value);
            nodes[static_cast<std::size_t>(index)].left = child;
        } else {
            const int child = update_add(nodes[static_cast<std::size_t>(index)].right, middle + 1, right, position, value);
            nodes[static_cast<std::size_t>(index)].right = child;
        }

        pull(index, left, right);
        if (!nodes[static_cast<std::size_t>(index)].left &&
            !nodes[static_cast<std::size_t>(index)].right) {
            release_subtree(index);
            return 0;
        }
        return index;
    }

    bool can_prune(int index) const {
        const Node& node = nodes[static_cast<std::size_t>(index)];
        return node.mark == 0;
    }

    int update_add_range(int index, std::int64_t left, std::int64_t right, std::int64_t query_left, std::int64_t query_right, const T& value) {
        if (value == T()) return index;
        if (!index) index = allocate_node();
        if (query_left <= left && right <= query_right) {
            apply_add(index, left, right, value);
            if (can_prune(index)) {
                release_subtree(index);
                return 0;
            }
            return index;
        }

        push(index, left, right);
        const std::int64_t middle = midpoint(left, right);
        if (query_left <= middle) {
            const int child = update_add_range(nodes[static_cast<std::size_t>(index)].left, left, middle, query_left, query_right, value);
            nodes[static_cast<std::size_t>(index)].left = child;
        }
        if (query_right > middle) {
            const int child = update_add_range(nodes[static_cast<std::size_t>(index)].right, middle + 1, right, query_left, query_right, value);
            nodes[static_cast<std::size_t>(index)].right = child;
        }
        pull(index, left, right);
        if (can_prune(index)) {
            release_subtree(index);
            return 0;
        }
        return index;
    }

    int update_assign_range(int index, std::int64_t left, std::int64_t right, std::int64_t query_left, std::int64_t query_right, const T& value) {
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
        const std::int64_t middle = midpoint(left, right);
        if (query_left <= middle) {
            const int child = update_assign_range(nodes[static_cast<std::size_t>(index)].left, left, middle, query_left, query_right, value);
            nodes[static_cast<std::size_t>(index)].left = child;
        }
        if (query_right > middle) {
            const int child = update_assign_range(nodes[static_cast<std::size_t>(index)].right, middle + 1, right, query_left, query_right, value);
            nodes[static_cast<std::size_t>(index)].right = child;
        }
        pull(index, left, right);
        if (can_prune(index)) {
            release_subtree(index);
            return 0;
        }
        return index;
    }

    Info full_info(int index, std::int64_t left, std::int64_t right, bool carry_assignment, const T& carry_assign_value, const T& carry_add) const {
        Info result;
        result.valid = true;
        if (index) {
            const Node& node = nodes[static_cast<std::size_t>(index)];
            result.sum = node.sum;
            result.minimum = node.minimum;
            result.maximum = node.maximum;
            result.mark = node.mark;
            result.minimum_count = node.minimum_count;
        } else {
            result = zero_info(left, right);
        }

        if (carry_assignment) {
            const std::uint64_t length = segment_length(left, right);
            result.sum = sum_for(carry_assign_value, left, right);
            result.minimum = carry_assign_value;
            result.maximum = carry_assign_value;
            result.mark = carry_assign_value == T() ? 0 : length;
            result.minimum_count = length;
        } else if (carry_add != T()) {
            const T old_minimum = result.minimum;
            const std::uint64_t length = segment_length(left, right);
            result.sum += sum_for(carry_add, left, right);
            result.minimum += carry_add;
            result.maximum += carry_add;
            if (carry_add > T()) {
                if (old_minimum == T()) result.mark = length;
            } else if (result.minimum == T()) {
                result.mark -= result.minimum_count;
            }
        }
        return result;
    }

    Info query_rec(int index, std::int64_t left, std::int64_t right,
                   std::int64_t query_left, std::int64_t query_right,
                   bool carry_assignment, const T& carry_assign_value,
                   const T& carry_add) const {
        if (query_right < left || right < query_left) return {};
        if (query_left <= left && right <= query_right) {
            return full_info(index, left, right, carry_assignment,
                             carry_assign_value, carry_add);
        }

        bool next_assignment = carry_assignment;
        T next_assign_value = carry_assign_value;
        T next_add = carry_add;
        if (!carry_assignment && index) {
            const Node& node = nodes[static_cast<std::size_t>(index)];
            if (node.has_assignment) {
                next_assignment = true;
                next_assign_value = node.assign_tag + carry_add;
                next_add = T();
            } else {
                next_add = node.add_tag + carry_add;
            }
        }

        const std::int64_t middle = midpoint(left, right);
        const int child_left = index ? nodes[static_cast<std::size_t>(index)].left : 0;
        const int child_right = index ? nodes[static_cast<std::size_t>(index)].right : 0;
        if (query_right <= middle) {
            return query_rec(child_left, left, middle, query_left, query_right,
                             next_assignment, next_assign_value, next_add);
        }
        if (query_left > middle) {
            return query_rec(child_right, middle + 1, right, query_left, query_right,
                             next_assignment, next_assign_value, next_add);
        }
        return merge(query_rec(child_left, left, middle, query_left, query_right,
                               next_assignment, next_assign_value, next_add),
                     query_rec(child_right, middle + 1, right, query_left, query_right,
                               next_assignment, next_assign_value, next_add));
    }

    Info query_info(std::int64_t left, std::int64_t right) const {
        return query_rec(root, domain_left, domain_right, left, right, false, T(), T());
    }

    static Info merge(const Info& left, const Info& right) {
        if (!left.valid) return right;
        if (!right.valid) return left;
        Info result;
        result.valid = true;
        result.sum = left.sum + right.sum;
        result.maximum = std::max(left.maximum, right.maximum);
        result.mark = left.mark + right.mark;
        result.minimum = std::min(left.minimum, right.minimum);
        result.minimum_count = (left.minimum == result.minimum ? left.minimum_count : 0) + (right.minimum == result.minimum ? right.minimum_count : 0);
        return result;
    }

    void compose_child_tags(int index, bool carry_assignment, const T& carry_assign_value, const T& carry_add, bool& next_assignment, T& next_assign_value, T& next_add) const {
        next_assignment = carry_assignment;
        next_assign_value = carry_assign_value;
        next_add = carry_add;
        if (!carry_assignment && index) {
            const Node& node = nodes[static_cast<std::size_t>(index)];
            if (node.has_assignment) {
                next_assignment = true;
                next_assign_value = node.assign_tag + carry_add;
                next_add = T();
            } else {
                next_add = node.add_tag + carry_add;
            }
        }
    }

    void collect_pieces(int index, std::int64_t left, std::int64_t right, std::int64_t query_left, std::int64_t query_right,
                        bool carry_assignment, const T& carry_assign_value, const T& carry_add, Piece* pieces, int& piece_count) const {
        if (query_right < left || right < query_left) return;
        if (query_left <= left && right <= query_right) {
            pieces[piece_count++] = {index, left, right, carry_assignment, carry_assign_value, carry_add};
            return;
        }
        bool next_assignment;
        T next_assign_value;
        T next_add;
        compose_child_tags(index, carry_assignment, carry_assign_value, carry_add, next_assignment, next_assign_value, next_add);
        const std::int64_t middle = midpoint(left, right);
        const int child_left = index ? nodes[static_cast<std::size_t>(index)].left : 0;
        const int child_right = index ? nodes[static_cast<std::size_t>(index)].right : 0;
        collect_pieces(child_left, left, middle, query_left, query_right, next_assignment, next_assign_value, next_add, pieces, piece_count);
        collect_pieces(child_right, middle + 1, right, query_left, query_right, next_assignment, next_assign_value, next_add, pieces, piece_count);
    }

    std::int64_t descend_by_sum(Piece piece, T order) const {
        int index = piece.node;
        std::int64_t left = piece.left;
        std::int64_t right = piece.right;
        bool carry_assignment = piece.carry_assignment;
        T carry_assign_value = piece.carry_assign_value;
        T carry_add = piece.carry_add;
        while (left < right) {
            const std::int64_t middle = midpoint(left, right);
            bool next_assignment;
            T next_assign_value;
            T next_add;
            compose_child_tags(index, carry_assignment, carry_assign_value, carry_add, next_assignment, next_assign_value, next_add);
            const int child_left = index ? nodes[static_cast<std::size_t>(index)].left : 0;
            const T left_sum = full_info(child_left, left, middle, next_assignment, next_assign_value, next_add).sum;
            if (order <= left_sum) {
                index = child_left;
                right = middle;
            } else {
                order -= left_sum;
                index = index ? nodes[static_cast<std::size_t>(index)].right : 0;
                left = middle + 1;
            }
            carry_assignment = next_assignment;
            carry_assign_value = next_assign_value;
            carry_add = next_add;
        }
        return left;
    }

    std::int64_t descend_by_mark(Piece piece, std::uint64_t order) const {
        int index = piece.node;
        std::int64_t left = piece.left;
        std::int64_t right = piece.right;
        bool carry_assignment = piece.carry_assignment;
        T carry_assign_value = piece.carry_assign_value;
        T carry_add = piece.carry_add;
        while (left < right) {
            const std::int64_t middle = midpoint(left, right);
            bool next_assignment;
            T next_assign_value;
            T next_add;
            compose_child_tags(index, carry_assignment, carry_assign_value, carry_add, next_assignment, next_assign_value, next_add);
            const int child_left = index ? nodes[static_cast<std::size_t>(index)].left : 0;
            const std::uint64_t left_mark = full_info(child_left, left, middle, next_assignment, next_assign_value, next_add).mark;
            if (order <= left_mark) {
                index = child_left;
                right = middle;
            } else {
                order -= left_mark;
                index = index ? nodes[static_cast<std::size_t>(index)].right : 0;
                left = middle + 1;
            }
            carry_assignment = next_assignment;
            carry_assign_value = next_assign_value;
            carry_add = next_add;
        }
        return left;
    }

    template <bool ByMark, class Order, class Aggregate>
    std::optional<std::int64_t> find_order_impl(std::int64_t left, std::int64_t right, Order order, Aggregate aggregate) const {
        Piece pieces[max_cover_pieces];
        int piece_count = 0;
        collect_pieces(root, domain_left, domain_right, left, right, false, T(), T(), pieces, piece_count);
        for (int i = 0; i < piece_count; ++i) {
            const Piece& piece = pieces[i];
            const Info info = full_info(piece.node, piece.left, piece.right, piece.carry_assignment, piece.carry_assign_value, piece.carry_add);
            const auto amount = aggregate(info);
            if (order > amount) {
                order -= amount;
            } else {
                if constexpr (ByMark) {
                    return descend_by_mark(piece, static_cast<std::uint64_t>(order));
                } else {
                    return descend_by_sum(piece, static_cast<T>(order));
                }
            }
        }
        return std::nullopt;
    }

public:
    Segment_tree() = default;

    void set_domain(std::int64_t left, std::int64_t right) {
#ifdef ZZ_LIBRARY_DEBUG
        if (left > right) {
            throw std::invalid_argument("zz_library::Segment_tree< T, domain >: invalid domain");
        }
#endif
        if (left == std::numeric_limits<std::int64_t>::min() &&
            right == std::numeric_limits<std::int64_t>::max()) {
            throw std::length_error("zz_library::Segment_tree< T, domain >: domain length exceeds uint64_t");
        }
        std::vector<Node>().swap(nodes);
        nodes.emplace_back();
        std::vector<int>().swap(free_nodes);
        root = 0;
        domain_left = left;
        domain_right = right;
        initialized = true;
    }

    void add(std::int64_t position, const T& value) {
#ifdef ZZ_LIBRARY_DEBUG
        check_position(position);
        if (count(position) + value < T()) {
            throw std::invalid_argument("zz_library::Segment_tree< T, domain >::add: negative resulting weight");
        }
#endif
        root = update_add(root, domain_left, domain_right, position, value);
    }

    void set(std::int64_t position, const T& value) {
#ifdef ZZ_LIBRARY_DEBUG
        check_position(position);
        if (value < T()) {
            throw std::invalid_argument("zz_library::Segment_tree< T, domain >::set: negative weight");
        }
#endif
        root = update_set(root, domain_left, domain_right, position, value);
    }

    void update(std::int64_t left, std::int64_t right, const T& value) {
#ifdef ZZ_LIBRARY_DEBUG
        check_range(left, right);
        if (query_info(left, right).minimum + value < T()) {
            throw std::invalid_argument("zz_library::Segment_tree< T, domain >::update: negative resulting weight");
        }
#endif
        root = update_add_range(root, domain_left, domain_right, left, right, value);
    }

    void assign(std::int64_t left, std::int64_t right, const T& value) {
#ifdef ZZ_LIBRARY_DEBUG
        check_range(left, right);
        if (value < T()) {
            throw std::invalid_argument("zz_library::Segment_tree< T, domain >::assign: negative weight");
        }
#endif
        root = update_assign_range(root, domain_left, domain_right, left, right, value);
    }

    std::optional<std::int64_t> find_by_order(std::int64_t left, std::int64_t right, T order) const {
#ifdef ZZ_LIBRARY_DEBUG
        check_range(left, right);
        if (order <= T()) {
            throw std::out_of_range("zz_library::Segment_tree< T, domain >::find_by_order: order must be positive");
        }
        if (query_info(left, right).minimum < T()) {
            throw std::invalid_argument("zz_library::Segment_tree< T, domain >::find_by_order: negative weights");
        }
#endif
        return find_order_impl<false>(left, right, order,
                                      [](const Info& info) { return info.sum; });
    }

    std::optional<std::int64_t> find_by_rank(std::int64_t left, std::int64_t right, std::uint64_t rank) const {
#ifdef ZZ_LIBRARY_DEBUG
        check_range(left, right);
        if (!rank) {
            throw std::out_of_range("zz_library::Segment_tree< T, domain >::find_by_rank: rank must be positive");
        }
#endif
        return find_order_impl<true>(left, right, rank,
                                     [](const Info& info) { return info.mark; });
    }

    T query_sum(std::int64_t left, std::int64_t right) const {
#ifdef ZZ_LIBRARY_DEBUG
        check_range(left, right);
#endif
        return query_info(left, right).sum;
    }

    T query_max(std::int64_t left, std::int64_t right) const {
#ifdef ZZ_LIBRARY_DEBUG
        check_range(left, right);
#endif
        return query_info(left, right).maximum;
    }

    std::uint64_t query_mark(std::int64_t left, std::int64_t right) const {
#ifdef ZZ_LIBRARY_DEBUG
        check_range(left, right);
#endif
        return query_info(left, right).mark;
    }

    std::optional<std::int64_t> upper_bound(std::int64_t left, std::int64_t right, std::int64_t value) const {
#ifdef ZZ_LIBRARY_DEBUG
        check_range(left, right);
#endif
        if (value >= right) return std::nullopt;
        const std::int64_t first = value < left ? left : value + 1;
        return find_order_impl<true>(first, right, std::uint64_t(1), [](const Info& info) { return info.mark; });
    }

    std::optional<std::int64_t> lower_bound(std::int64_t left, std::int64_t right, std::int64_t value) const {
#ifdef ZZ_LIBRARY_DEBUG
        check_range(left, right);
#endif
        if (value > right) return std::nullopt;
        const std::int64_t first = value < left ? left : value;
        return find_order_impl<true>(first, right, std::uint64_t(1), [](const Info& info) { return info.mark; });
    }

    T count_less(std::int64_t left, std::int64_t right, std::int64_t value) const {
#ifdef ZZ_LIBRARY_DEBUG
        check_range(left, right);
#endif
        if (value <= left) return T();
        if (value > right) return query_info(left, right).sum;
        return query_info(left, value - 1).sum;
    }

    T count_greater(std::int64_t left, std::int64_t right, std::int64_t value) const {
#ifdef ZZ_LIBRARY_DEBUG
        check_range(left, right);
#endif
        if (value < left) return query_info(left, right).sum;
        if (value >= right) return T();
        return query_info(value + 1, right).sum;
    }

    T count(std::int64_t position) const {
#ifdef ZZ_LIBRARY_DEBUG
        check_position(position);
#endif
        return query_info(position, position).sum;
    }

    std::optional<std::int64_t> get_min() const {
#ifdef ZZ_LIBRARY_DEBUG
        check_initialized();
#endif
        return find_order_impl<true>(domain_left, domain_right, std::uint64_t(1), [](const Info& info) { return info.mark; });
    }

    std::optional<std::int64_t> get_max() const {
#ifdef ZZ_LIBRARY_DEBUG
        check_initialized();
#endif
        const std::uint64_t total_mark = query_info(domain_left, domain_right).mark;
        if (!total_mark) return std::nullopt;
        return find_order_impl<true>(domain_left, domain_right, total_mark, [](const Info& info) { return info.mark; });
    }
};

} // namespace zz_library

#endif // ZZ_LIBRARY_SEG_TREE_DOMAIN_HPP
