#ifndef ZZ_LIBRARY_FENWICK_HPP
#define ZZ_LIBRARY_FENWICK_HPP

#if defined(LOCAL) && !defined(ZZ_LIBRARY_DEBUG)
#define ZZ_LIBRARY_DEBUG
#endif

namespace zz_library {

template <class T>
class fenwick_tree {
private:
    int n = 0;
    std::vector<T> bit1;
    std::vector<T> bit2;

#ifdef ZZ_LIBRARY_DEBUG
    void check_index(int index) const {
        if (index < 0 || index >= n) {
            throw std::out_of_range("zz_library::Fenwick_tree: index out of range");
        }
    }

    void check_range(int left, int right) const {
        if (left < 0 || right >= n || left > right) {
            throw std::out_of_range("zz_library::Fenwick_tree: invalid range");
        }
    }
#endif

    void add_to_bit(std::vector<T>& bit, int index, const T& value) {
        for (; index <= n; index += index & -index) {
            bit[static_cast<std::size_t>(index)] += value;
        }
    }

    T bit_sum(const std::vector<T>& bit, int index) const {
        T result{};
        for (; index > 0; index -= index & -index) {
            result += bit[static_cast<std::size_t>(index)];
        }
        return result;
    }

    // Difference Fenwick trees: prefix_sum(k) = k * sum(bit1, k) - sum(bit2, k).
    T prefix_sum(int count) const {
        return T(count) * bit_sum(bit1, count) - bit_sum(bit2, count);
    }

    void add_range(int left, int right, const T& value) {
        const int start = left + 1;
        add_to_bit(bit1, start, value);
        add_to_bit(bit2, start, value * T(left));

        const int after = right + 2;
        if (after <= n) {
            add_to_bit(bit1, after, -value);
            add_to_bit(bit2, after, -value * T(right + 1));
        }
    }

public:
    fenwick_tree() = default;

    void init(const std::vector<T>& values) {
        n = static_cast<int>(values.size());
        bit1.assign(static_cast<std::size_t>(n) + 1, T());
        bit2.assign(static_cast<std::size_t>(n) + 1, T());

        for (int index = 1; index <= n; ++index) {
            const T difference = values[static_cast<std::size_t>(index - 1)] - (index == 1 ? T() : values[static_cast<std::size_t>(index - 2)]);
            bit1[static_cast<std::size_t>(index)] += difference;
            bit2[static_cast<std::size_t>(index)] += difference * T(index - 1);

            const int parent = index + (index & -index);
            if (parent <= n) {
                bit1[static_cast<std::size_t>(parent)] += bit1[static_cast<std::size_t>(index)];
                bit2[static_cast<std::size_t>(parent)] += bit2[static_cast<std::size_t>(index)];
            }
        }
    }

    void set(int index, const T& value) {
#ifdef ZZ_LIBRARY_DEBUG
        check_index(index);
#endif
        const T current = prefix_sum(index + 1) - prefix_sum(index);
        add_range(index, index, value - current);
    }

    void add(int index, const T& value) {
#ifdef ZZ_LIBRARY_DEBUG
        check_index(index);
#endif
        add_range(index, index, value);
    }

    void update(int left, int right, const T& value) {
#ifdef ZZ_LIBRARY_DEBUG
        check_range(left, right);
#endif
        add_range(left, right, value);
    }

    T query(int left, int right) const {
#ifdef ZZ_LIBRARY_DEBUG
        check_range(left, right);
#endif
        return prefix_sum(right + 1) - prefix_sum(left);
    }
};

} // namespace zz_library

#endif // ZZ_LIBRARY_FENWICK_HPP
