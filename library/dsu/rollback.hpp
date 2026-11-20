#ifndef ZZ_LIBRARY_DISJOINT_SET_ROLLBACK_HPP
#define ZZ_LIBRARY_DISJOINT_SET_ROLLBACK_HPP
#include "core.hpp"
namespace zz_library {
    template<class T>
    class disjoint_set<T, rollback, void> {
        struct Change {
            T child, parent, old_size;
        };
        std::vector<T> parent, comp_size;
        std::vector<Change> history;
        T components = 0;
        T root(T x) const {
            while (parent[static_cast<std::size_t>(x)] != x) x = parent[static_cast<std::size_t>(x)];
            return x;
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
            for (std::size_t i = 0; i < m; ++i) parent[i] = static_cast<T>(i);
            components = static_cast<T>(m);
            history.clear();
        }
        void reset() {
            parent.clear();
            comp_size.clear();
            history.clear();
            components = 0;
        }
        T find(T x) const {
            detail::dsu_index<T>::check(x, parent.size());
            return root(x);
        }
        bool check(T x, T y) const {
            return find(x) == find(y);
        }
        bool merge(T x, T y) {
            x = find(x);
            y = find(y);
            if (x == y) return false;
            if (comp_size[static_cast<std::size_t>(x)] < comp_size[static_cast<std::size_t>(y)]) std::swap(x, y);
            history.push_back({y, x, comp_size[static_cast<std::size_t>(x)]});
            parent[static_cast<std::size_t>(y)] = x;
            comp_size[static_cast<std::size_t>(x)] += comp_size[static_cast<std::size_t>(y)];
            --components;
            return true;
        }
        T count(T x) const {
            return comp_size[static_cast<std::size_t>(find(x))];
        }
        T size() const {
            return components;
        }
        std::size_t snapshot() const {
            return history.size();
        }
        bool undo() {
            if (history.empty()) return false;
            Change c = history.back();
            history.pop_back();
            parent[static_cast<std::size_t>(c.child)] = c.child;
            comp_size[static_cast<std::size_t>(c.parent)] = c.old_size;
            ++components;
            return true;
        }
        void undo(std::size_t snap) {
            while (history.size() > snap) undo();
        }
    };
}
#endif
