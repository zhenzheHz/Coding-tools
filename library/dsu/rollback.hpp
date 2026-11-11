#ifndef ZZ_LIBRARY_DISJOINT_SET_ROLLBACK_HPP
#define ZZ_LIBRARY_DISJOINT_SET_ROLLBACK_HPP

#include "core.hpp"

namespace zz_library {

template<class T>
struct disjoint_set<T, rollback> {
    static_assert(std::is_integral_v<T> && std::is_signed_v<T>);

private:
    struct History {
        T child;
        T parent;
        T parent_size;
    };
    std::vector<T> parent;
    std::vector<T> component_size;
    std::vector<History> history;
    T component_count = 0;

    void check_index(T x) const {
#if defined(LOCAL) || defined(ZZ_LIBRARY_DEBUG)
        if (x < 0 || static_cast<std::size_t>(x) >= parent.size()) { throw std::out_of_range("disjoint_set::undo: index out of range"); }
#endif
    }

public:
    disjoint_set() = default;
    explicit disjoint_set(T n) { init(n); }

    void init(T n) {
#if defined(LOCAL) || defined(ZZ_LIBRARY_DEBUG)
        if (n < 0 || n == std::numeric_limits<T>::max()) { throw std::invalid_argument("disjoint_set::undo: invalid size"); }
#endif
        std::size_t length = static_cast<std::size_t>(n) + 1;
        parent.resize(length);
        component_size.assign(length, 1);
        history.clear();
        for (std::size_t i = 0; i < length; ++i) { parent[i] = static_cast<T>(i); }
        component_count = static_cast<T>(length);
    }

    void reset() {
        parent.clear();
        component_size.clear();
        history.clear();
        component_count = 0;
    }

    T size() const {
        return component_count;
    }

    T find(T x) const {
        check_index(x);
        while (parent[x] != x) x = parent[x];
        return x;
    }

    bool check(T x, T y) const {
        return find(x) == find(y);
    }

    bool merge(T x, T y) {
        x = find(x), y = find(y);
        if (x == y) return false;
        if (component_size[x] < component_size[y]) std::swap(x, y);
        history.push_back({y, x, component_size[x]});
        parent[y] = x;
        component_size[x] += component_size[y];
        --component_count;
        return true;
    }

    T count(T x) const {
        return component_size[find(x)];
    }

    std::size_t snapshot() const {
        return history.size();
    }

    void undo() {
#if defined(LOCAL) || defined(ZZ_LIBRARY_DEBUG)
        if (history.empty()) { throw std::logic_error("disjoint_set::undo: no history to undo"); }
#endif
        const History record = history.back();
        history.pop_back();
        parent[record.child] = record.child;
        component_size[record.parent] = record.parent_size;
        ++component_count;
    }

    void undo(std::size_t snap) {
#if defined(LOCAL) || defined(ZZ_LIBRARY_DEBUG)
        if (snap > history.size()) { throw std::out_of_range("disjoint_set::undo: invalid snapshot"); }
#endif
        while (history.size() > snap) undo();
    }
};

}

#endif
