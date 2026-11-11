#ifndef ZZ_LIBRARY_DISJOINT_SET_BASIC_HPP
#define ZZ_LIBRARY_DISJOINT_SET_BASIC_HPP

#include "core.hpp"

namespace zz_library {

template<class T>
struct disjoint_set<T, basic> {
    static_assert(std::is_integral_v<T> && std::is_signed_v<T>);

    std::vector<T> parent, component_size;
    T component_count = 0;
    T largest_size = 0;

    disjoint_set() = default;

    explicit disjoint_set(T n) {
        init(n);
    }

    void init(T n) {
#if defined(LOCAL) || defined(ZZ_LIBRARY_DEBUG)
        if (n < 0) throw std::invalid_argument("invalid size");
#endif
        parent.resize(static_cast<std::size_t>(n) + 1);
        component_size.assign(parent.size(), 1);
        std::iota(parent.begin(), parent.end(), T{0});
        component_count = static_cast<T>(parent.size());
        largest_size = parent.empty() ? 0 : 1;
    }

    void reset() {
        parent.clear();
        component_size.clear();
        component_count = 0;
        largest_size = 0;
    }

    T size() const {
        return component_count;
    }

    T find(T x) {
        check_index(x);

        T root = x;
        while (parent[root] != root) {
            root = parent[root];
        }

        while (parent[x] != x) {
            T p = parent[x];
            parent[x] = root;
            x = p;
        }

        return root;
    }

    bool check(T x, T y) {
        return find(x) == find(y);
    }

    bool merge(T x, T y) {
        x = find(x), y = find(y);
        if (x == y) return false;
        if (component_size[x] < component_size[y]) {
            std::swap(x, y);
        }
        parent[y] = x;
        component_size[x] += component_size[y];
        largest_size = std::max(largest_size, component_size[x]);
        --component_count;
        return true;
    }

    T count(T x) {
        return component_size[find(x)];
    }

    T max_size() const {
        return largest_size;
    }

    std::vector<T> group(T x) {
        T root = find(x);
        std::vector<T> result;
        result.reserve(static_cast<std::size_t>(component_size[root]));
        for (std::size_t i = 0; i < parent.size(); ++i) {
            T v = static_cast<T>(i);
            if (find(v) == root) {
                result.push_back(v);
            }
        }
        return result;
    }

private:
    void check_index(T x) const {
#if defined(LOCAL) || defined(ZZ_LIBRARY_DEBUG)
        if (x < 0 || static_cast<std::size_t>(x) >= parent.size()) {
            throw std::out_of_range("disjoint_set index out of range");
        }
#else
        static_cast<void>(x);
#endif
    }
};

}

#endif
