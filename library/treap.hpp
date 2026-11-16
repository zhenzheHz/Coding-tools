#ifndef ZZ_LIBRARY_TREAP_HPP
#define ZZ_LIBRARY_TREAP_HPP

#include "int128.hpp"

namespace zz_library {

struct implicit {};
struct ordered {};

template<typename T> class implicit_treap;
template<typename T> class ordered_treap;

template<typename T, typename Mode = implicit>
struct treap_selector {
    static_assert(std::is_same_v<Mode, implicit> || std::is_same_v<Mode, ordered>, "Unsupported treap mode");
};

template<typename T>
struct treap_selector<T, implicit> { using type = implicit_treap<T>; };

template<typename T>
struct treap_selector<T, ordered> { using type = ordered_treap<T>; };

template<typename T, typename Mode = implicit>
using treap = typename treap_selector<T, Mode>::type;

namespace detail {

template<typename T, typename = void>
struct treap_has_less : std::false_type {};

template<typename T>
struct treap_has_less<T, std::void_t<decltype(std::declval<const T&>() < std::declval<const T&>())>> : std::true_type {};

template<typename T, typename = void>
struct treap_has_affine : std::false_type {};

template<typename T>
struct treap_has_affine<T, std::void_t<decltype(T(std::declval<std::int64_t>()) * std::declval<T>() + T(std::declval<std::int64_t>()))>> : std::true_type {};

template<typename T, typename = void>
struct treap_sum_type { using type = T; };

template<typename T>
struct treap_sum_type<T, std::void_t<decltype(std::declval<T>() + std::declval<T>())>> {
    using type = decltype(std::declval<T>() + std::declval<T>());
};

template<typename T, typename = void>
struct treap_wide_type { using type = T; };

template<typename T>
struct treap_wide_type<T, std::enable_if_t<std::is_integral_v<T> && sizeof(T) <= 4>> { using type = std::int64_t; };

template<typename T>
struct treap_wide_type<T, std::enable_if_t<std::is_integral_v<T> && sizeof(T) == 8 && std::is_signed_v<T>>> { using type = __int128_t; };

template<typename T>
struct treap_wide_type<T, std::enable_if_t<std::is_integral_v<T> && sizeof(T) == 8 && std::is_unsigned_v<T>>> { using type = __uint128_t; };

template<> struct treap_wide_type<float, void> { using type = double; };
template<> struct treap_wide_type<double, void> { using type = double; };
template<> struct treap_wide_type<long double, void> { using type = long double; };

template<typename T>
using treap_wide_type_t = typename treap_wide_type<T>::type;

template<typename T>
using treap_affine_type_t = std::conditional_t<std::is_integral_v<T>, __int128_t, std::conditional_t<std::is_floating_point_v<T>, long double, std::conditional_t<treap_has_affine<T>::value, T, std::int64_t>>>;

template<typename T>
using treap_sum_type_t = std::conditional_t<std::is_integral_v<T>, treap_wide_type_t<T>, typename treap_sum_type<T>::type>;

template<typename T>
inline constexpr bool treap_mod_supported_v = std::is_integral_v<T> && sizeof(T) <= sizeof(std::int64_t);

template<typename T>
inline constexpr bool treap_minmax_supported_v = treap_has_less<T>::value;

inline std::int64_t treap_mod_norm(__int128_t x, std::int64_t mod) {
    x %= mod;
    if (x < 0) x += mod;
    return static_cast<std::int64_t>(x);
}

}

template<typename T>
class implicit_treap {
public:
    using value_type = T;
    using size_type = std::size_t;
    using sum_type = detail::treap_sum_type_t<T>;
    using wide_type = detail::treap_wide_type_t<T>;
    using affine_type = detail::treap_affine_type_t<T>;

private:
    struct EmptyExtrema {};
    struct Extrema { T minimum{}, maximum{}; };
    using extrema_type = std::conditional_t<detail::treap_minmax_supported_v<T>, Extrema, EmptyExtrema>;

    struct Node {
        T value{};
        std::uint64_t priority = 0;
        size_type left = 0, right = 0, size = 1;
        bool reversed = false;
        bool affine_tag = false;
        affine_type lazy_a{1}, lazy_b{0};
        sum_type sum{};
        extrema_type extrema{};

        Node() = default;
        Node(const T& v, std::uint64_t p) : value(v), priority(p), sum(v) {
            if constexpr (detail::treap_minmax_supported_v<T>) extrema.minimum = extrema.maximum = v;
        }
    };

    mutable std::vector<Node> pool;
    std::vector<size_type> free_list;
    std::mt19937_64 rng{std::random_device{}()};
    size_type root = 0;
    bool seed_set = false;
    bool mod_set = false;
    bool mod_locked = false;
    bool has_ever_inserted = false;
    std::int64_t mod = 0;
    std::uint64_t version = 0;

    size_type node_size(size_type x) const { return x ? pool[x].size : 0; }
    sum_type node_sum(size_type x) const { return x ? pool[x].sum : sum_type{}; }

    size_type allocate_node(const T& value) {
        std::uint64_t p = rng();
        if (!free_list.empty()) {
            size_type x = free_list.back();
            free_list.pop_back();
            pool[x] = Node(value, p);
            return x;
        }
        pool.emplace_back(value, p);
        return pool.size() - 1;
    }

    void release_node(size_type x) {
        pool[x].left = pool[x].right = 0;
        pool[x].size = 1;
        pool[x].reversed = false;
        pool[x].affine_tag = false;
        pool[x].lazy_a = affine_type(1);
        pool[x].lazy_b = affine_type(0);
        free_list.push_back(x);
    }

    void release_subtree(size_type x) {
        if (!x) return;
        size_type l = pool[x].left, r = pool[x].right;
        release_subtree(l);
        release_subtree(r);
        release_node(x);
    }

    void pull(size_type x) {
        Node& n = pool[x];
        n.size = 1 + node_size(n.left) + node_size(n.right);
        if constexpr (detail::treap_mod_supported_v<T>) {
            if (mod_set) {
                __int128_t total = static_cast<__int128_t>(node_sum(n.left)) + static_cast<__int128_t>(n.value) + static_cast<__int128_t>(node_sum(n.right));
                n.sum = static_cast<sum_type>(detail::treap_mod_norm(total, mod));
            } else {
                n.sum = node_sum(n.left) + static_cast<sum_type>(n.value) + node_sum(n.right);
            }
        } else {
            n.sum = node_sum(n.left) + static_cast<sum_type>(n.value) + node_sum(n.right);
        }
        if constexpr (detail::treap_minmax_supported_v<T>) {
            n.extrema.minimum = n.extrema.maximum = n.value;
            if (n.left) {
                if (pool[n.left].extrema.minimum < n.extrema.minimum) n.extrema.minimum = pool[n.left].extrema.minimum;
                if (n.extrema.maximum < pool[n.left].extrema.maximum) n.extrema.maximum = pool[n.left].extrema.maximum;
            }
            if (n.right) {
                if (pool[n.right].extrema.minimum < n.extrema.minimum) n.extrema.minimum = pool[n.right].extrema.minimum;
                if (n.extrema.maximum < pool[n.right].extrema.maximum) n.extrema.maximum = pool[n.right].extrema.maximum;
            }
        }
    }

    void apply_reverse(size_type x) const {
        if (!x) return;
        std::swap(pool[x].left, pool[x].right);
        pool[x].reversed = !pool[x].reversed;
    }

    void apply_affine(size_type x, affine_type a, affine_type b) {
        if (!x) return;
        if constexpr (!detail::treap_has_affine<T>::value && !std::is_arithmetic_v<T>) {
            throw std::invalid_argument("treap: affine is unsupported for this value type");
        } else {
            Node& n = pool[x];
            if constexpr (detail::treap_mod_supported_v<T>) {
                if (mod_set) {
                    std::int64_t aa = detail::treap_mod_norm(static_cast<__int128_t>(a), mod);
                    std::int64_t bb = detail::treap_mod_norm(static_cast<__int128_t>(b), mod);
                    std::int64_t xx = detail::treap_mod_norm(static_cast<__int128_t>(n.value), mod);
                    n.value = static_cast<T>(detail::treap_mod_norm(static_cast<__int128_t>(aa) * xx + bb, mod));
                    n.sum = static_cast<sum_type>(detail::treap_mod_norm(static_cast<__int128_t>(aa) * static_cast<__int128_t>(n.sum) + static_cast<__int128_t>(bb) * n.size, mod));
                    n.lazy_a = detail::treap_mod_norm(static_cast<__int128_t>(aa) * n.lazy_a, mod);
                    n.lazy_b = detail::treap_mod_norm(static_cast<__int128_t>(aa) * n.lazy_b + bb, mod);
                    n.affine_tag = true;
                    return;
                }
            }
            if constexpr (std::is_arithmetic_v<T>) {
                n.value = static_cast<T>(a * static_cast<affine_type>(n.value) + b);
                n.sum = static_cast<sum_type>(a * static_cast<affine_type>(n.sum) + b * n.size);
                if constexpr (detail::treap_minmax_supported_v<T>) {
                    T old_min = n.extrema.minimum, old_max = n.extrema.maximum;
                    if (a > 0) {
                        n.extrema.minimum = static_cast<T>(a * static_cast<affine_type>(old_min) + b);
                        n.extrema.maximum = static_cast<T>(a * static_cast<affine_type>(old_max) + b);
                    } else if (a < 0) {
                        n.extrema.minimum = static_cast<T>(a * static_cast<affine_type>(old_max) + b);
                        n.extrema.maximum = static_cast<T>(a * static_cast<affine_type>(old_min) + b);
                    } else n.extrema.minimum = n.extrema.maximum = static_cast<T>(b);
                }
            } else {
                n.value = a * n.value + b;
                n.sum = a * n.sum + b * static_cast<T>(n.size);
                if constexpr (detail::treap_minmax_supported_v<T>) {
                    T old_min = n.extrema.minimum, old_max = n.extrema.maximum, zero(0);
                    if (zero < a) {
                        n.extrema.minimum = a * old_min + b;
                        n.extrema.maximum = a * old_max + b;
                    } else if (a < zero) {
                        n.extrema.minimum = a * old_max + b;
                        n.extrema.maximum = a * old_min + b;
                    } else n.extrema.minimum = n.extrema.maximum = b;
                }
            }
            n.lazy_a = a * n.lazy_a;
            n.lazy_b = a * n.lazy_b + b;
            n.affine_tag = true;
        }
    }

    void push(size_type x) const {
        if (!x) return;
        if (pool[x].reversed) {
            apply_reverse(pool[x].left);
            apply_reverse(pool[x].right);
            pool[x].reversed = false;
        }
        if (pool[x].affine_tag) {
            const_cast<implicit_treap*>(this)->apply_affine(pool[x].left, pool[x].lazy_a, pool[x].lazy_b);
            const_cast<implicit_treap*>(this)->apply_affine(pool[x].right, pool[x].lazy_a, pool[x].lazy_b);
            pool[x].affine_tag = false;
            pool[x].lazy_a = affine_type(1);
            pool[x].lazy_b = affine_type(0);
        }
    }

    std::pair<size_type, size_type> split(size_type x, size_type k) {
        if (!x) return {0, 0};
        push(x);
        if (node_size(pool[x].left) >= k) {
            auto [a, b] = split(pool[x].left, k);
            pool[x].left = b;
            pull(x);
            return {a, x};
        }
        auto [a, b] = split(pool[x].right, k - node_size(pool[x].left) - 1);
        pool[x].right = a;
        pull(x);
        return {x, b};
    }

    size_type merge(size_type a, size_type b) {
        if (!a) return b;
        if (!b) return a;
        if (pool[a].priority > pool[b].priority) {
            push(a);
            pool[a].right = merge(pool[a].right, b);
            pull(a);
            return a;
        }
        push(b);
        pool[b].left = merge(a, pool[b].left);
        pull(b);
        return b;
    }

    size_type kth(size_type x, size_type k) const {
        while (x) {
            push(x);
            size_type ls = node_size(pool[x].left);
            if (k < ls) x = pool[x].left;
            else if (k == ls) return x;
            else { k -= ls + 1; x = pool[x].right; }
        }
        return 0;
    }

    void set_at(size_type x, size_type k, const T& value) {
        push(x);
        size_type ls = node_size(pool[x].left);
        if (k < ls) set_at(pool[x].left, k, value);
        else if (k == ls) pool[x].value = value;
        else set_at(pool[x].right, k - ls - 1, value);
        pull(x);
    }

    void build(const std::vector<T>& v) {
        if (mod_set) for (const T& value : v) check_mod_value(value);
        pool.resize(1);
        free_list.clear();
        root = 0;
        ++version;
        if (v.empty()) return;
        std::vector<size_type> st;
        st.reserve(v.size());
        for (const T& value : v) {
            size_type x = allocate_node(value), last = 0;
            while (!st.empty() && pool[st.back()].priority < pool[x].priority) { last = st.back(); st.pop_back(); }
            pool[x].left = last;
            if (!st.empty()) pool[st.back()].right = x;
            st.push_back(x);
        }
        root = st.front();
        mod_locked = true;
        has_ever_inserted = true;
        std::vector<size_type> order{root};
        for (size_type i = 0; i < order.size(); ++i) {
            size_type x = order[i];
            if (pool[x].left) order.push_back(pool[x].left);
            if (pool[x].right) order.push_back(pool[x].right);
        }
        for (size_type i = order.size(); i-- > 0;) pull(order[i]);
        ++version;
    }

    void check_mod_value(const T& x) const {
#ifdef LOCAL
        if constexpr (detail::treap_mod_supported_v<T>) {
            if (static_cast<__int128_t>(x) < 0 || static_cast<__int128_t>(x) >= mod)
                throw std::invalid_argument("treap: value outside modulo range");
        }
#else
        (void)x;
#endif
    }

    void check_position(size_type p) const {
        if (p >= size()) throw std::out_of_range("treap: position out of range");
    }

    void check_interval(size_type l, size_type r) const {
        if (l > r) throw std::invalid_argument("treap: invalid interval");
        if (r >= size()) throw std::out_of_range("treap: interval out of range");
    }

public:
    class iterator {
        friend class implicit_treap;
        const implicit_treap* owner = nullptr;
        size_type rank = 0;
        std::uint64_t saved_version = 0;
        iterator(const implicit_treap* p, size_type r) : owner(p), rank(r), saved_version(p ? p->version : 0) {}
        void validate() const {
#ifdef LOCAL
            if (owner && saved_version != owner->version) throw std::logic_error("treap: invalidated iterator");
#endif
        }
    public:
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = void;
        using reference = T;
        iterator() = default;
        T operator*() const { validate(); if (!owner || rank >= owner->size()) throw std::out_of_range("treap: dereferencing end iterator"); return owner->at(rank); }
        iterator& operator++() { validate(); ++rank; return *this; }
        iterator operator++(int) { iterator t = *this; ++*this; return t; }
        iterator& operator--() { validate(); --rank; return *this; }
        iterator operator--(int) { iterator t = *this; --*this; return t; }
        bool operator==(const iterator& other) const { return owner == other.owner && rank == other.rank; }
        bool operator!=(const iterator& other) const { return !(*this == other); }
    };

    implicit_treap() : pool(1) {}
    explicit implicit_treap(size_type n) : pool(1) { init(std::vector<T>(n)); }
    implicit_treap(size_type n, const T& value) : pool(1) { init(std::vector<T>(n, value)); }
    explicit implicit_treap(const std::vector<T>& v) : pool(1) { init(v); }
    implicit_treap(const implicit_treap&) = default;
    implicit_treap& operator=(const implicit_treap& other) {
        if (this == &other) return *this;
        pool = other.pool;
        free_list = other.free_list;
        rng = other.rng;
        root = other.root;
        seed_set = other.seed_set;
        mod_set = other.mod_set;
        mod_locked = other.mod_locked;
        has_ever_inserted = other.has_ever_inserted;
        mod = other.mod;
        ++version;
        return *this;
    }
    implicit_treap(implicit_treap&& other) : pool(std::move(other.pool)), free_list(std::move(other.free_list)), rng(std::move(other.rng)), root(other.root), seed_set(other.seed_set), mod_set(other.mod_set), mod_locked(other.mod_locked), has_ever_inserted(other.has_ever_inserted), mod(other.mod) {
        other.pool.clear();
        other.pool.emplace_back();
        other.free_list.clear();
        other.root = 0;
        other.mod_set = false;
        other.mod_locked = false;
        other.mod = 0;
        other.has_ever_inserted = false;
        other.seed_set = false;
        ++other.version;
    }
    implicit_treap& operator=(implicit_treap&& other) {
        if (this == &other) return *this;
        pool = std::move(other.pool);
        free_list = std::move(other.free_list);
        rng = std::move(other.rng);
        root = other.root;
        seed_set = other.seed_set;
        mod_set = other.mod_set;
        mod_locked = other.mod_locked;
        has_ever_inserted = other.has_ever_inserted;
        mod = other.mod;
        ++version;
        other.pool.clear();
        other.pool.emplace_back();
        other.free_list.clear();
        other.root = 0;
        other.mod_set = false;
        other.mod_locked = false;
        other.mod = 0;
        other.has_ever_inserted = false;
        other.seed_set = false;
        ++other.version;
        return *this;
    }

    void set_seed(std::uint64_t seed) {
        if (seed_set || has_ever_inserted) throw std::invalid_argument("treap: seed can only be set once before insertion");
        rng.seed(seed);
        seed_set = true;
    }

    void set_mod(std::int64_t m) {
        if (m <= 0) throw std::invalid_argument("treap: modulo must be positive");
        if (mod_locked || mod_set) throw std::logic_error("treap: modulo can only be set once before insertion");
        if constexpr (!detail::treap_mod_supported_v<T>) {
            throw std::invalid_argument("treap: modulo requires an integral type up to 64 bits");
        } else {
            if (static_cast<__int128_t>(m) > static_cast<__int128_t>(std::numeric_limits<T>::max()) + 1)
                throw std::invalid_argument("treap: modulo exceeds the value type range");
        }
        mod = m;
        mod_set = true;
    }

    void init(const std::vector<T>& v) { build(v); }
    size_type size() const { return node_size(root); }
    bool empty() const { return root == 0; }
    void clear() { pool.resize(1); free_list.clear(); root = 0; mod_set = false; mod_locked = false; mod = 0; ++version; }
    void reserve(size_type n) { pool.reserve(n + 1); }

    T at(size_type pos) const { check_position(pos); return pool[kth(root, pos)].value; }
    T operator[](size_type pos) const { return at(pos); }
    void set(size_type pos, const T& value) {
        check_position(pos);
        if (mod_set) check_mod_value(value);
        set_at(root, pos, value);
        ++version;
    }

    void insert(size_type pos, const T& value) {
        if (pos > size()) throw std::out_of_range("treap: insertion position out of range");
        if (mod_set) check_mod_value(value);
        size_type x = allocate_node(value);
        auto [a, b] = split(root, pos);
        root = merge(merge(a, x), b);
        has_ever_inserted = true;
        mod_locked = true;
        ++version;
    }

    void erase(size_type pos) {
        check_position(pos);
        auto [a, b] = split(root, pos);
        auto [mid, c] = split(b, 1);
        root = merge(a, c);
        release_node(mid);
        ++version;
    }

    void erase_interval(size_type l, size_type r) {
        check_interval(l, r);
        auto [a, b] = split(root, l);
        auto [mid, c] = split(b, r - l + 1);
        release_subtree(mid);
        root = merge(a, c);
        ++version;
    }

    void push_back(const T& x) { insert(size(), x); }
    void push_front(const T& x) { insert(0, x); }
    void pop_back() { if (empty()) throw std::out_of_range("treap: pop_back of empty treap"); erase(size() - 1); }
    void pop_front() { if (empty()) throw std::out_of_range("treap: pop_front of empty treap"); erase(0); }
    T front() const { if (empty()) throw std::out_of_range("treap: front of empty treap"); return at(0); }
    T back() const { if (empty()) throw std::out_of_range("treap: back of empty treap"); return at(size() - 1); }

    void reverse(size_type l, size_type r) {
        check_interval(l, r);
        auto [a, b] = split(root, l);
        auto [mid, c] = split(b, r - l + 1);
        apply_reverse(mid);
        root = merge(merge(a, mid), c);
        ++version;
    }

    void rotate(size_type l, size_type r, size_type m) {
        if (l > m || m >= r) throw std::invalid_argument("treap: invalid rotation");
        if (r >= size()) throw std::out_of_range("treap: interval out of range");
        auto [a, b] = split(root, l);
        auto [mid, c] = split(b, r - l + 1);
        auto [x, y] = split(mid, m - l + 1);
        root = merge(merge(merge(a, y), x), c);
        ++version;
    }

    void splice(size_type l, size_type r, size_type p) {
        check_interval(l, r);
        if (p > size()) throw std::out_of_range("treap: splice position out of range");
        if (p >= l && p <= r + 1) return;
        size_type len = r - l + 1;
        auto [a, b] = split(root, l);
        auto [mid, c] = split(b, len);
        root = merge(a, c);
        if (p > r) p -= len;
        auto [x, y] = split(root, p);
        root = merge(merge(x, mid), y);
        ++version;
    }

    void affine(size_type l, size_type r, std::int64_t a, std::int64_t b) {
        check_interval(l, r);
        if constexpr (!std::is_arithmetic_v<T> && !detail::treap_has_affine<T>::value) {
            throw std::invalid_argument("treap: affine is unsupported for this value type");
        }
        auto [x, y] = split(root, l);
        auto [mid, z] = split(y, r - l + 1);
        apply_affine(mid, static_cast<affine_type>(a), static_cast<affine_type>(b));
        root = merge(merge(x, mid), z);
        ++version;
    }

    sum_type query_sum(size_type l, size_type r) {
        check_interval(l, r);
        auto [a, b] = split(root, l);
        auto [mid, c] = split(b, r - l + 1);
        sum_type ans = node_sum(mid);
        root = merge(merge(a, mid), c);
        if constexpr (detail::treap_mod_supported_v<T>) {
            if (mod_set) return static_cast<sum_type>(detail::treap_mod_norm(static_cast<__int128_t>(ans), mod));
        }
        return ans;
    }

    T query_min(size_type l, size_type r) {
        if (mod_set) throw std::invalid_argument("treap: min is unavailable with modulo");
        if constexpr (!detail::treap_minmax_supported_v<T>) {
            throw std::invalid_argument("treap: min requires operator<");
        } else {
            check_interval(l, r);
            auto [a, b] = split(root, l);
            auto [mid, c] = split(b, r - l + 1);
            T ans = pool[mid].extrema.minimum;
            root = merge(merge(a, mid), c);
            return ans;
        }
    }

    T query_max(size_type l, size_type r) {
        if (mod_set) throw std::invalid_argument("treap: max is unavailable with modulo");
        if constexpr (!detail::treap_minmax_supported_v<T>) {
            throw std::invalid_argument("treap: max requires operator<");
        } else {
            check_interval(l, r);
            auto [a, b] = split(root, l);
            auto [mid, c] = split(b, r - l + 1);
            T ans = pool[mid].extrema.maximum;
            root = merge(merge(a, mid), c);
            return ans;
        }
    }

    std::vector<T> to_vector() const {
        std::vector<T> result;
        result.reserve(size());
        std::vector<size_type> st;
        size_type x = root;
        while (x || !st.empty()) {
            while (x) { push(x); st.push_back(x); x = pool[x].left; }
            x = st.back(); st.pop_back();
            result.push_back(pool[x].value);
            x = pool[x].right;
        }
        return result;
    }

    template<typename F>
    void for_each(F f) const {
        std::vector<size_type> st;
        size_type x = root;
        while (x || !st.empty()) {
            while (x) { push(x); st.push_back(x); x = pool[x].left; }
            x = st.back(); st.pop_back();
            f(pool[x].value);
            x = pool[x].right;
        }
    }

    iterator begin() const { return iterator(this, 0); }
    iterator end() const { return iterator(this, size()); }
};

template<typename T>
class ordered_treap {
public:
    using value_type = T;
    using size_type = std::size_t;

private:
    static_assert(detail::treap_has_less<T>::value, "ordered_treap requires operator<");

    struct Node {
        T value{};
        std::uint64_t priority = 0;
        size_type left = 0, right = 0, count = 1, size = 1;
        Node() = default;
        Node(const T& v, std::uint64_t p) : value(v), priority(p) {}
    };

    std::vector<Node> pool{1};
    std::vector<size_type> free_list;
    std::mt19937_64 rng{std::random_device{}()};
    size_type root = 0;
    bool seed_set = false;
    bool has_ever_inserted = false;
    std::uint64_t version = 0;

    size_type node_size(size_type x) const { return x ? pool[x].size : 0; }
    void pull(size_type x) { pool[x].size = pool[x].count + node_size(pool[x].left) + node_size(pool[x].right); }
    size_type allocate_node(const T& v) {
        std::uint64_t p = rng();
        if (!free_list.empty()) { size_type x = free_list.back(); free_list.pop_back(); pool[x] = Node(v, p); return x; }
        pool.emplace_back(v, p);
        return pool.size() - 1;
    }
    void release_node(size_type x) { pool[x].left = pool[x].right = 0; pool[x].count = pool[x].size = 1; free_list.push_back(x); }
    std::pair<size_type, size_type> split_less(size_type x, const T& key) {
        if (!x) return {0, 0};
        if (pool[x].value < key) {
            auto [a, b] = split_less(pool[x].right, key);
            pool[x].right = a; pull(x); return {x, b};
        }
        auto [a, b] = split_less(pool[x].left, key);
        pool[x].left = b; pull(x); return {a, x};
    }
    std::pair<size_type, size_type> split_leq(size_type x, const T& key) {
        if (!x) return {0, 0};
        if (!(key < pool[x].value)) {
            auto [a, b] = split_leq(pool[x].right, key);
            pool[x].right = a; pull(x); return {x, b};
        }
        auto [a, b] = split_leq(pool[x].left, key);
        pool[x].left = b; pull(x); return {a, x};
    }
    size_type merge(size_type a, size_type b) {
        if (!a) return b;
        if (!b) return a;
        if (pool[a].priority > pool[b].priority) { pool[a].right = merge(pool[a].right, b); pull(a); return a; }
        pool[b].left = merge(a, pool[b].left); pull(b); return b;
    }
    size_type find_node(const T& x) const {
        size_type u = root;
        while (u) {
            if (pool[u].value < x) u = pool[u].right;
            else if (x < pool[u].value) u = pool[u].left;
            else return u;
        }
        return 0;
    }
    size_type kth(size_type x, size_type k) const {
        while (x) {
            size_type ls = node_size(pool[x].left);
            if (k < ls) x = pool[x].left;
            else if (k < ls + pool[x].count) return x;
            else { k -= ls + pool[x].count; x = pool[x].right; }
        }
        return 0;
    }

    T value_by_order(size_type k) const { return pool[kth(root, k)].value; }

public:
    class iterator {
        friend class ordered_treap;
        const ordered_treap* owner = nullptr;
        size_type rank = 0;
        std::uint64_t saved_version = 0;
        iterator(const ordered_treap* p, size_type r) : owner(p), rank(r), saved_version(p ? p->version : 0) {}
        void validate() const {
#ifdef LOCAL
            if (owner && saved_version != owner->version) throw std::logic_error("treap: invalidated iterator");
#endif
        }
    public:
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = void;
        using reference = T;
        iterator() = default;
        T operator*() const { validate(); if (!owner || rank >= owner->size()) throw std::out_of_range("treap: dereferencing end iterator"); return owner->value_by_order(rank); }
        iterator& operator++() { validate(); ++rank; return *this; }
        iterator operator++(int) { iterator t = *this; ++*this; return t; }
        iterator& operator--() { validate(); --rank; return *this; }
        iterator operator--(int) { iterator t = *this; --*this; return t; }
        bool operator==(const iterator& other) const { return owner == other.owner && rank == other.rank; }
        bool operator!=(const iterator& other) const { return !(*this == other); }
    };

    ordered_treap() = default;
    explicit ordered_treap(const std::vector<T>& v) { init(v); }
    ordered_treap(const ordered_treap&) = default;
    ordered_treap& operator=(const ordered_treap& other) {
        if (this == &other) return *this;
        pool = other.pool;
        free_list = other.free_list;
        rng = other.rng;
        root = other.root;
        seed_set = other.seed_set;
        has_ever_inserted = other.has_ever_inserted;
        ++version;
        return *this;
    }
    ordered_treap(ordered_treap&& other) : pool(std::move(other.pool)), free_list(std::move(other.free_list)), rng(std::move(other.rng)), root(other.root), seed_set(other.seed_set), has_ever_inserted(other.has_ever_inserted) {
        other.pool.clear();
        other.pool.emplace_back();
        other.free_list.clear();
        other.root = 0;
        other.seed_set = false;
        other.has_ever_inserted = false;
        ++other.version;
    }
    ordered_treap& operator=(ordered_treap&& other) {
        if (this == &other) return *this;
        pool = std::move(other.pool);
        free_list = std::move(other.free_list);
        rng = std::move(other.rng);
        root = other.root;
        seed_set = other.seed_set;
        has_ever_inserted = other.has_ever_inserted;
        ++version;
        other.pool.clear();
        other.pool.emplace_back();
        other.free_list.clear();
        other.root = 0;
        other.seed_set = false;
        other.has_ever_inserted = false;
        ++other.version;
        return *this;
    }

    void set_seed(std::uint64_t seed) { if (seed_set || has_ever_inserted) throw std::invalid_argument("treap: seed can only be set once before insertion"); rng.seed(seed); seed_set = true; }
    size_type size() const { return node_size(root); }
    bool empty() const { return root == 0; }
    void clear() { pool.resize(1); free_list.clear(); root = 0; ++version; }
    void reserve(size_type n) { pool.reserve(n + 1); }

    void init(const std::vector<T>& v) {
        clear();
        std::vector<T> a = v;
        std::sort(a.begin(), a.end());
        for (const T& x : a) insert(x);
    }

    void insert(const T& x) {
        size_type u = find_node(x);
        if (u) ++pool[u].count;
        else {
            size_type node = allocate_node(x);
            auto [a, b] = split_less(root, x);
            root = merge(merge(a, node), b);
        }
        if (u) {
            std::vector<size_type> path;
            size_type cur = root;
            while (cur) { path.push_back(cur); if (pool[cur].value < x) cur = pool[cur].right; else if (x < pool[cur].value) cur = pool[cur].left; else break; }
            for (size_type i = path.size(); i-- > 0;) pull(path[i]);
        }
        has_ever_inserted = true;
        ++version;
    }

    bool erase_one(const T& x) {
        size_type u = find_node(x);
        if (!u) return false;
        if (pool[u].count > 1) {
            --pool[u].count;
            std::vector<size_type> path;
            size_type cur = root;
            while (cur) { path.push_back(cur); if (pool[cur].value < x) cur = pool[cur].right; else if (x < pool[cur].value) cur = pool[cur].left; else break; }
            for (size_type i = path.size(); i-- > 0;) pull(path[i]);
        } else {
            auto [a, b] = split_less(root, x);
            auto [mid, c] = split_leq(b, x);
            root = merge(a, c);
            release_node(mid);
        }
        ++version;
        return true;
    }

    size_type erase_all(const T& x) {
        size_type u = find_node(x);
        if (!u) return 0;
        size_type result = pool[u].count;
        auto [a, b] = split_less(root, x);
        auto [mid, c] = split_leq(b, x);
        root = merge(a, c);
        release_node(mid);
        ++version;
        return result;
    }

    iterator find(const T& x) const { size_type k = order_of_key(x); return iterator(this, contains(x) ? k : size()); }
    bool contains(const T& x) const { return find_node(x) != 0; }
    size_type count(const T& x) const { size_type u = find_node(x); return u ? pool[u].count : 0; }
    iterator lower_bound(const T& x) const { size_type k = order_of_key(x); return iterator(this, k); }
    iterator upper_bound(const T& x) const { size_type k = order_of_key(x) + count(x); return iterator(this, k); }
    iterator find_by_order(size_type k) const { return iterator(this, k < size() ? k : size()); }
    size_type order_of_key(const T& x) const {
        size_type ans = 0, u = root;
        while (u) { if (pool[u].value < x) { ans += node_size(pool[u].left) + pool[u].count; u = pool[u].right; } else u = pool[u].left; }
        return ans;
    }
    T front() const { if (empty()) throw std::out_of_range("treap: front of empty treap"); return value_by_order(0); }
    T back() const { if (empty()) throw std::out_of_range("treap: back of empty treap"); return value_by_order(size() - 1); }
    iterator begin() const { return iterator(this, 0); }
    iterator end() const { return iterator(this, size()); }
};

}

#endif
