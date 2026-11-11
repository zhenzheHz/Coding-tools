
#ifndef ZZ_LIBRARY_SPARSE_TABLE_HPP
#define ZZ_LIBRARY_SPARSE_TABLE_HPP

namespace zz_library {

struct RMQ {};
struct GCD {};
struct AND {};
struct OR {};

namespace detail {

template<class T>
struct sparse_table_min {
    T operator()(const T &a, const T &b) const {
        return a < b ? a : b;
    }
};

template<class T>
struct sparse_table_max {
    T operator()(const T &a, const T &b) const {
        return a < b ? b : a;
    }
};

template<class T>
struct sparse_table_gcd {
    T operator()(const T &a, const T &b) const {
        return std::gcd(a, b);
    }
};

template<class T>
struct sparse_table_and {
    T operator()(const T &a, const T &b) const {
        return a & b;
    }
};

template<class T>
struct sparse_table_or {
    T operator()(const T &a, const T &b) const {
        return a | b;
    }
};

template<class T, class Op>
class sparse_table_base {
private:
    std::size_t n = 0;
    std::vector<std::size_t> lg;
    std::vector<std::vector<T>> table;

    void check(std::size_t l, std::size_t r) const {
#if defined(LOCAL) || defined(ZZ_LIBRARY_DEBUG)
        if (l > r || r >= n) { throw std::out_of_range("sparse_table: invalid range"); }
#else
        (void)l; (void)r;
#endif
    }

public:
    sparse_table_base() = default;

    explicit sparse_table_base(const std::vector<T> &v) {
        init(v);
    }

    void init(const std::vector<T> &v) {
        n = v.size();
        lg.assign(n + 1, 0);
        table.clear();
        if (n == 0) return;
        for (std::size_t i = 2; i <= n; ++i) {
            lg[i] = lg[i / 2] + 1;
        }
        std::size_t levels = lg[n] + 1;
        table.resize(levels);
        table[0] = v;
        for (std::size_t k = 1; k < levels; ++k) {
            std::size_t len = std::size_t(1) << k;
            std::size_t half = len >> 1;
            table[k].resize(n - len + 1);
            for (std::size_t i = 0; i + len <= n; ++i) {
                table[k][i] = Op{}(table[k - 1][i], table[k - 1][i + half]);
            }
        }
    }

    T query(std::size_t l, std::size_t r) const {
        check(l, r);
        std::size_t k = lg[r - l + 1];
        return Op{}(table[k][l], table[k][r - (std::size_t(1) << k) + 1]);
    }

    std::size_t size() const {
        return n;
    }

    bool empty() const {
        return n == 0;
    }
};

}

template<class T, class Type>
class sparse_table;

template<class T>
class sparse_table<T, RMQ> {
private:
    detail::sparse_table_base<T, detail::sparse_table_min<T>> table_min;
    detail::sparse_table_base<T, detail::sparse_table_max<T>> table_max;

public:
    sparse_table() = default;

    explicit sparse_table(const std::vector<T> &v) {
        init(v);
    }

    void init(const std::vector<T> &v) {
        table_min.init(v);
        table_max.init(v);
    }

    T query_min(std::size_t l, std::size_t r) const {
        return table_min.query(l, r);
    }

    T query_max(std::size_t l, std::size_t r) const {
        return table_max.query(l, r);
    }

    std::size_t size() const {
        return table_min.size();
    }

    bool empty() const {
        return table_min.empty();
    }
};

template<class T>
class sparse_table<T, GCD> : public detail::sparse_table_base<T, detail::sparse_table_gcd<T>> {
    using base = detail::sparse_table_base<T, detail::sparse_table_gcd<T>>;

public:
    using base::base;
};

template<class T>
class sparse_table<T, AND> : public detail::sparse_table_base<T, detail::sparse_table_and<T>> {
    using base = detail::sparse_table_base<T, detail::sparse_table_and<T>>;

public:
    using base::base;
};

template<class T>
class sparse_table<T, OR> : public detail::sparse_table_base<T, detail::sparse_table_or<T>> {
    using base = detail::sparse_table_base<T, detail::sparse_table_or<T>>;

public:
    using base::base;
};

}

#endif
