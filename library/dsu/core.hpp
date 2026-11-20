#ifndef ZZ_LIBRARY_DISJOINT_SET_CORE_HPP
#define ZZ_LIBRARY_DISJOINT_SET_CORE_HPP

namespace zz_library {

struct basic {};
struct almost {};
struct rollback {};
struct potential {};
struct persistent {};

template<class T, class Type = basic, class Group = void>
class disjoint_set;

template<class T>
struct additive_group {
    using value_type = T;
    static value_type identity() { return value_type(); }
    static value_type op(const value_type& a, const value_type& b) { return a + b; }
    static value_type inv(const value_type& a) { return -a; }
};

namespace detail {

template<class T>
struct dsu_index {
    static_assert(std::is_integral<T>::value && std::is_signed<T>::value, "DSU index type must be a signed integral type");
    static std::size_t checked_size(T n) {
#ifdef LOCAL
        if (n < 0) throw std::invalid_argument("negative DSU size");
#endif
        return static_cast<std::size_t>(n) + 1;
    }
    static void check(T x, std::size_t n) {
#ifdef LOCAL
        if (x < 0 || static_cast<std::size_t>(x) >= n) throw std::out_of_range("DSU index out of range");
#else
        (void)x; (void)n;
#endif
    }
};

}
}

#endif
