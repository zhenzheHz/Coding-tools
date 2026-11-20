#ifndef ZZ_LIBRARY_DISJOINT_SET_ALMOST_HPP
#define ZZ_LIBRARY_DISJOINT_SET_ALMOST_HPP
#include "core.hpp"
namespace zz_library {
    template<class T>
    class disjoint_set<T, almost, void> {
        using sum_type = typename std::conditional<(sizeof(T) < 8), std::int64_t, T>::type;
        struct Node {
            T parent, size, head, tail;
            sum_type sum;
        };
        std::vector<Node> nodes;
        std::vector<T> id, prev, next;
        T component_count = 0;
        T root(T x) {
            T& p = nodes[static_cast<std::size_t>(x)].parent;
            return p == x ? x : p = root(p);
        }
        T new_node(T x) {
            T p = static_cast<T>(nodes.size());
            nodes.push_back({p, 1, x, x, static_cast<sum_type>(x)});
            return p;
        }
        void detach(T x, T r) {
            T p = prev[static_cast<std::size_t>(x)], q = next[static_cast<std::size_t>(x)];
            if (p == -1) nodes[static_cast<std::size_t>(r)].head = q;
            else next[static_cast<std::size_t>(p)] = q;
            if (q == -1) nodes[static_cast<std::size_t>(r)].tail = p;
            else prev[static_cast<std::size_t>(q)] = p;
            prev[static_cast<std::size_t>(x)] = next[static_cast<std::size_t>(x)] = -1;
            --nodes[static_cast<std::size_t>(r)].size;
            nodes[static_cast<std::size_t>(r)].sum -= static_cast<sum_type>(x);
        }
        void append(T x, T r) {
            T tail = nodes[static_cast<std::size_t>(r)].tail;
            prev[static_cast<std::size_t>(x)] = tail;
            next[static_cast<std::size_t>(x)] = -1;
            if (tail == -1) nodes[static_cast<std::size_t>(r)].head = x;
            else next[static_cast<std::size_t>(tail)] = x;
            nodes[static_cast<std::size_t>(r)].tail = x;
            ++nodes[static_cast<std::size_t>(r)].size;
            nodes[static_cast<std::size_t>(r)].sum += static_cast<sum_type>(x);
        }
    public:
        disjoint_set() = default;
        explicit disjoint_set(T n) {
            init(n);
        }
        void init(T n) {
            std::size_t m = detail::dsu_index<T>::checked_size(n);
            nodes.clear();
            id.resize(m);
            prev.assign(m, -1);
            next.assign(m, -1);
            nodes.reserve(m);
            for (std::size_t i = 0; i < m; ++i) {
                id[i] = static_cast<T>(i);
                nodes.push_back({static_cast<T>(i), 1, static_cast<T>(i), static_cast<T>(i), static_cast<sum_type>(i)});
            }
            component_count = static_cast<T>(m);
        }
        void reset() {
            nodes.clear();
            id.clear();
            prev.clear();
            next.clear();
            component_count = 0;
        }
        T size() const {
            return static_cast<T>(id.size());
        }
        T components() const {
            return component_count;
        }
        T find(T x) {
            detail::dsu_index<T>::check(x, id.size());
            return nodes[static_cast<std::size_t>(root(id[static_cast<std::size_t>(x)]))].head;
        }
        bool check(T x, T y) {
            detail::dsu_index<T>::check(x, id.size());
            detail::dsu_index<T>::check(y, id.size());
            return root(id[static_cast<std::size_t>(x)]) == root(id[static_cast<std::size_t>(y)]);
        }
        bool merge(T x, T y) {
            detail::dsu_index<T>::check(x, id.size());
            detail::dsu_index<T>::check(y, id.size());
            x = root(id[static_cast<std::size_t>(x)]);
            y = root(id[static_cast<std::size_t>(y)]);
            if (x == y) return false;
            if (nodes[static_cast<std::size_t>(x)].size < nodes[static_cast<std::size_t>(y)].size) std::swap(x, y);
            Node& a = nodes[static_cast<std::size_t>(x)];
            Node& b = nodes[static_cast<std::size_t>(y)];
            next[static_cast<std::size_t>(a.tail)] = b.head;
            prev[static_cast<std::size_t>(b.head)] = a.tail;
            a.tail = b.tail;
            a.size += b.size;
            a.sum += b.sum;
            b.parent = x;
            --component_count;
            return true;
        }
        bool shift(T x, T y) {
            detail::dsu_index<T>::check(x, id.size());
            detail::dsu_index<T>::check(y, id.size());
            T a = root(id[static_cast<std::size_t>(x)]), b = root(id[static_cast<std::size_t>(y)]);
            if (a == b) return false;
            detach(x, a);
            if (nodes[static_cast<std::size_t>(a)].size == 0) --component_count;
            T fresh = new_node(x);
            id[static_cast<std::size_t>(x)] = fresh;
            nodes[static_cast<std::size_t>(fresh)].parent = b;
            nodes[static_cast<std::size_t>(fresh)].size = 0;
            nodes[static_cast<std::size_t>(fresh)].head = nodes[static_cast<std::size_t>(fresh)].tail = -1;
            nodes[static_cast<std::size_t>(fresh)].sum = 0;
            append(x, b);
            return true;
        }
        bool reset(T x) {
            detail::dsu_index<T>::check(x, id.size());
            T a = root(id[static_cast<std::size_t>(x)]);
            if (nodes[static_cast<std::size_t>(a)].size == 1) return false;
            detach(x, a);
            T fresh = new_node(x);
            id[static_cast<std::size_t>(x)] = fresh;
            ++component_count;
            return true;
        }
        T count(T x) {
            return nodes[static_cast<std::size_t>(root(id[static_cast<std::size_t>(x)]))].size;
        }
        sum_type get_sum(T x) {
            return nodes[static_cast<std::size_t>(root(id[static_cast<std::size_t>(x)]))].sum;
        }
        std::vector<T> group(T x) {
            T r = root(id[static_cast<std::size_t>(x)]);
            std::vector<T> result;
            for (std::size_t i = 0; i < id.size(); ++i) if (root(id[i]) == r) result.push_back(static_cast<T>(i));
            return result;
        }
    };
}
#endif
