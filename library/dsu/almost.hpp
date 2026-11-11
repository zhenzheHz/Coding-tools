#ifndef ZZ_LIBRARY_DISJOINT_SET_ALMOST_HPP
#define ZZ_LIBRARY_DISJOINT_SET_ALMOST_HPP

#include "core.hpp"

namespace zz_library {

template<class T>
struct disjoint_set<T, almost> {
    static_assert(std::is_integral_v<T> && std::is_signed_v<T>);
    using sum_type = std::conditional_t<(sizeof(T) < sizeof(std::int64_t)), std::int64_t, T>;

private:
    std::vector<T> parent, component_size;
    std::vector<sum_type> component_sum;
    std::vector<T> id, head, tail;
    std::vector<T> prev_element, next_element;

    T element_count = 0;
    T component_count = 0;
    T node_count = 0;

    T find_root(T x) {
        T root = x;
        while (parent[root] != root) root = parent[root];
        while (parent[x] != x) {
            T p = parent[x];
            parent[x] = root;
            x = p;
        }
        return root;
    }

    T create_node(T x) {
        T node = node_count++;
        parent.push_back(node);
        component_size.push_back(1);
        component_sum.push_back(x);
        head.push_back(x);
        tail.push_back(x);
        return node;
    }

    void unlink_element(T x, T root) {
        T p = prev_element[x];
        T q = next_element[x];

        if (p == T{-1}) head[root] = q;
        else next_element[p] = q;

        if (q == T{-1}) tail[root] = p;
        else prev_element[q] = p;

        prev_element[x] = next_element[x] = T{-1};
    }

    void append_element(T x, T root) {
        prev_element[x] = tail[root];
        next_element[x] = T{-1};
        next_element[tail[root]] = x;
        tail[root] = x;
    }

    void check_index(T x) const {
#if defined(LOCAL) || defined(ZZ_LIBRARY_DEBUG)
        if (x < 0 || x >= element_count) { throw std::out_of_range("disjoint_set index out of range"); }
#else
        static_cast<void>(x);
#endif
    }

public:
    disjoint_set() = default;

    explicit disjoint_set(T n) {
        init(n);
    }

    // Initializes elements [0, n].
    void init(T n) {
#if defined(LOCAL) || defined(ZZ_LIBRARY_DEBUG)
        if (n < 0) throw std::invalid_argument("invalid size");
#endif
        std::size_t m = static_cast<std::size_t>(n) + 1;

        parent.resize(m);
        component_size.assign(m, 1);
        component_sum.resize(m);
        id.resize(m);
        head.resize(m);
        tail.resize(m);
        prev_element.assign(m, T{-1});
        next_element.assign(m, T{-1});

        std::iota(parent.begin(), parent.end(), T{0});
        std::iota(component_sum.begin(), component_sum.end(), sum_type{0});
        std::iota(id.begin(), id.end(), T{0});
        std::iota(head.begin(), head.end(), T{0});
        std::iota(tail.begin(), tail.end(), T{0});

        element_count = static_cast<T>(m);
        component_count = element_count;
        node_count = element_count;
    }

    // Clears all elements and allocated virtual nodes.
    void reset() {
        parent.clear();
        component_size.clear();
        component_sum.clear();
        id.clear();
        head.clear();
        tail.clear();
        prev_element.clear();
        next_element.clear();

        element_count = 0;
        component_count = 0;
        node_count = 0;
    }

    T size() const {
        return element_count;
    }

    T components() const {
        return component_count;
    }

    // Returns an original element in x's component.
    T find(T x) {
        check_index(x);
        return head[find_root(id[x])];
    }

    bool check(T x, T y) {
        check_index(x);
        check_index(y);
        return find_root(id[x]) == find_root(id[y]);
    }

    bool merge(T x, T y) {
        check_index(x); check_index(y);
        x = find_root(id[x]), y = find_root(id[y]);
        if (x == y) return false;
        if (component_size[x] < component_size[y]) std::swap(x, y);
        parent[y] = x;
        component_size[x] += component_size[y];
        component_sum[x] += component_sum[y];
        next_element[tail[x]] = head[y];
        prev_element[head[y]] = tail[x];
        tail[x] = tail[y];
        --component_count;
        return true;
    }

    // Moves x into y's component.
    bool shift(T x, T y) {
        check_index(x); check_index(y);
        T rx = find_root(id[x]), ry = find_root(id[y]);
        if (rx == ry) return false;
        unlink_element(x, rx);
        --component_size[rx];
        component_sum[rx] -= x;
        T node = create_node(x);
        id[x] = node;
        parent[node] = ry;
        ++component_size[ry];
        component_sum[ry] += x;
        append_element(x, ry);
        if (component_size[rx] == 0) --component_count;
        return true;
    }

    // Detaches x into a singleton component.
    bool reset(T x) {
        check_index(x);
        T root = find_root(id[x]);
        if (component_size[root] == 1) return false;
        unlink_element(x, root);
        --component_size[root];
        component_sum[root] -= x;
        T node = create_node(x);
        id[x] = node;
        ++component_count;
        return true;
    }

    T count(T x) {
        check_index(x);
        return component_size[find_root(id[x])];
    }

    sum_type get_sum(T x) {
        check_index(x);
        return component_sum[find_root(id[x])];
    }

    std::vector<T> group(T x) {
        check_index(x);
        T root = find_root(id[x]);
        std::vector<T> result;
        result.reserve(static_cast<std::size_t>(component_size[root]));
        for (T v = head[root]; v != T{-1}; v = next_element[v]) {
            result.push_back(v);
        }
        std::sort(result.begin(), result.end());
        return result;
    }
};

}

#endif
