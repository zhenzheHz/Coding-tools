#ifndef ZZ_LIBRARY_DISJOINT_SET_BASIC_HPP
#define ZZ_LIBRARY_DISJOINT_SET_BASIC_HPP
#include "core.hpp"
namespace zz_library {
    template<class T>
    class disjoint_set<T, basic, void> {
        std::vector<T> parent, comp_size;
        T components = 0, largest = 0;
        T root(T x) {
            return parent[static_cast<std::size_t>(x)] == x ? x : parent[static_cast<std::size_t>(x)] = root(parent[static_cast<std::size_t>(x)]);
        }
    public:
        using value_type = T;
        disjoint_set() = default;
        explicit disjoint_set(T n) {
            init(n);
        }
        void init(T n) {
            std::size_t m = detail::dsu_index<T>::checked_size(n);
            parent.resize(m);
            comp_size.assign(m, 1);
            for (std::size_t i = 0; i < m; ++i) parent[i] = static_cast<T>(i);
            components = static_cast<T>(m);
            largest = m ? 1 : 0;
        }
        void reset() {
            parent.clear();
            comp_size.clear();
            components = largest = 0;
        }
        T find(T x) {
            detail::dsu_index<T>::check(x, parent.size());
            return root(x);
        }
        bool check(T x, T y) {
            return find(x) == find(y);
        }
        bool merge(T x, T y) {
            x = find(x);
            y = find(y);
            if (x == y) return false;
            if (comp_size[static_cast<std::size_t>(x)] < comp_size[static_cast<std::size_t>(y)]) std::swap(x, y);
            parent[static_cast<std::size_t>(y)] = x;
            comp_size[static_cast<std::size_t>(x)] += comp_size[static_cast<std::size_t>(y)];
            largest = std::max(largest, comp_size[static_cast<std::size_t>(x)]);
            --components;
            return true;
        }
        T count_of(T x) {
            return comp_size[static_cast<std::size_t>(find(x))];
        }
        T count(T x) {
            return count_of(x);
        }
        T size() const {
            return components;
        }
        T max_size() const {
            return largest;
        }
        std::vector<T> group(T x) {
            T r = find(x);
            std::vector<T> result;
            for (std::size_t i = 0; i < parent.size(); ++i) if (root(static_cast<T>(i)) == r) result.push_back(static_cast<T>(i));
            return result;
        }
    };
}
#endif
