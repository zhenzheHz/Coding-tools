#ifndef ZZ_LIBRARY_DISJOINT_SET_POTENTIAL_HPP
#define ZZ_LIBRARY_DISJOINT_SET_POTENTIAL_HPP
#include "core.hpp"
namespace zz_library {
    template<class T, class Group>
    class disjoint_set<T, potential, Group> {
    public:
        using value_type = typename Group::value_type;
    private:
        std::vector<T> parent, comp_size;
        std::vector<value_type> weight;
        T components = 0;
        T root(T x) {
            if (parent[static_cast<std::size_t>(x)] == x) return x;
            T p = parent[static_cast<std::size_t>(x)];
            T r = root(p);
            weight[static_cast<std::size_t>(x)] = Group::op(weight[static_cast<std::size_t>(x)], weight[static_cast<std::size_t>(p)]);
            return parent[static_cast<std::size_t>(x)] = r;
        }
        value_type potential_to_root(T x) {
            root(x);
            return weight[static_cast<std::size_t>(x)];
        }
    public:
        disjoint_set() = default;
        explicit disjoint_set(T n) {
            init(n);
        }
        void init(T n) {
            std::size_t m = detail::dsu_index<T>::checked_size(n);
            parent.resize(m);
            comp_size.assign(m, 1);
            weight.assign(m, Group::identity());
            for (std::size_t i = 0; i < m; ++i) parent[i] = static_cast<T>(i);
            components = static_cast<T>(m);
        }
        void reset() {
            parent.clear();
            comp_size.clear();
            weight.clear();
            components = 0;
        }
        T find(T x) {
            detail::dsu_index<T>::check(x, parent.size());
            return root(x);
        }
        bool check(T x, T y) {
            return find(x) == find(y);
        }
        bool merge(T x, T y, const value_type& w) {
            T rx = find(x), ry = find(y);
            value_type px = weight[static_cast<std::size_t>(x)], py = weight[static_cast<std::size_t>(y)];
            if (rx == ry) return Group::op(py, Group::inv(px)) == w;
            if (comp_size[static_cast<std::size_t>(rx)] < comp_size[static_cast<std::size_t>(ry)]) {
                parent[static_cast<std::size_t>(rx)] = ry;
                weight[static_cast<std::size_t>(rx)] = Group::op(Group::op(Group::inv(px), Group::inv(w)), py);
                comp_size[static_cast<std::size_t>(ry)] += comp_size[static_cast<std::size_t>(rx)];
            }
            else {
                parent[static_cast<std::size_t>(ry)] = rx;
                weight[static_cast<std::size_t>(ry)] = Group::op(Group::op(Group::inv(py), w), px);
                comp_size[static_cast<std::size_t>(rx)] += comp_size[static_cast<std::size_t>(ry)];
            }
            --components;
            return true;
        }
        std::optional<value_type> diff(T x, T y) {
            if (find(x) != find(y)) return std::nullopt;
            return Group::op(weight[static_cast<std::size_t>(y)], Group::inv(weight[static_cast<std::size_t>(x)]));
        }
        T count(T x) {
            return comp_size[static_cast<std::size_t>(find(x))];
        }
        T size() const {
            return components;
        }
    };
    template<class T>
    class disjoint_set<T, potential, void> : public disjoint_set<T, potential, additive_group<T>> {
        using base = disjoint_set<T, potential, additive_group<T>>;
    public:
        using base::base;
    };
}
#endif
