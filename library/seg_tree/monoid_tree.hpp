#ifndef ZZ_LIBRARY_SEG_TREE_MONOID_TREE_HPP
#define ZZ_LIBRARY_SEG_TREE_MONOID_TREE_HPP

#include "core.hpp"

namespace zz_library {

template <class T, auto op, auto e>
class monoid_tree {
private:
    int n = 0;
    int tree_size = 1;
    std::vector<T> tree;

#ifdef ZZ_LIBRARY_DEBUG
    void check_index(int index) const {
        if (index < 0 || index >= n) {
            throw std::out_of_range("zz_library::monoid_tree: index out of range");
        }
    }

    void check_range(int left, int right) const {
        if (left < 0 || right >= n || left > right + 1) {
            throw std::out_of_range("zz_library::monoid_tree: invalid range");
        }
    }
#endif

public:
    monoid_tree() : tree(2, e()) {}

    explicit monoid_tree(const std::vector<T>& values) {
        init(values);
    }

    explicit monoid_tree(int n) {
        init(n);
    }

    void init(const std::vector<T>& values) {
        n = static_cast<int>(values.size());
        tree_size = 1;

        while (tree_size < n) tree_size <<= 1;

        tree.assign(tree_size << 1, e());

        for (int i = 0; i < n; ++i) {
            tree[tree_size + i] = values[static_cast<std::size_t>(i)];
        }

        for (int i = tree_size - 1; i >= 1; --i) {
            tree[i] = op(tree[i << 1], tree[i << 1 | 1]);
        }
    }

    void init(int n) {
        if (n < 0) throw std::invalid_argument("zz_library::monoid_tree: negative size");

        this->n = n;
        tree_size = 1;

        while (tree_size < n) tree_size <<= 1;

        tree.assign(tree_size << 1, e());

        for (int i = tree_size - 1; i >= 1; --i) {
            tree[i] = op(tree[i << 1], tree[i << 1 | 1]);
        }
    }

    void set(int index, const T& value) {
#ifdef ZZ_LIBRARY_DEBUG
        check_index(index);
#endif

        int p = index + tree_size;
        tree[p] = value;

        while (p >>= 1) {
            tree[p] = op(tree[p << 1], tree[p << 1 | 1]);
        }
    }

    T get(int index) const {
#ifdef ZZ_LIBRARY_DEBUG
        check_index(index);
#endif

        return tree[tree_size + index];
    }

    T prod(int left, int right) const {
#ifdef ZZ_LIBRARY_DEBUG
        check_range(left, right);
#endif

        T left_result = e();
        T right_result = e();

        left += tree_size;
        right += tree_size + 1;

        while (left < right) {
            if (left & 1) left_result = op(left_result, tree[left++]);
            if (right & 1) right_result = op(tree[--right], right_result);

            left >>= 1;
            right >>= 1;
        }

        return op(left_result, right_result);
    }

    T all_prod() const {
        return tree[1];
    }

    template <class F>
    int max_right(int left, F predicate) const {
#ifdef ZZ_LIBRARY_DEBUG
        if (left < 0 || left > n) {
            throw std::out_of_range("zz_library::monoid_tree: invalid left boundary");
        }
        if (!predicate(e())) {
            throw std::invalid_argument("zz_library::monoid_tree: predicate(e()) must be true");
        }
#endif

        if (left == n) return n - 1;

        int p = left + tree_size;
        T result = e();

        do {
            while (!(p & 1)) p >>= 1;

            T next = op(result, tree[p]);

            if (!predicate(next)) {
                while (p < tree_size) {
                    p <<= 1;
                    next = op(result, tree[p]);

                    if (predicate(next)) {
                        result = next;
                        ++p;
                    }
                }

                return p - tree_size - 1;
            }

            result = next;
            ++p;
        } while ((p & -p) != p);

        return n - 1;
    }

    template <class F>
    int min_left(int right, F predicate) const {
#ifdef ZZ_LIBRARY_DEBUG
        if (right < -1 || right >= n) {
            throw std::out_of_range("zz_library::monoid_tree: invalid right boundary");
        }
        if (!predicate(e())) {
            throw std::invalid_argument("zz_library::monoid_tree: predicate(e()) must be true");
        }
#endif

        if (right == -1) return 0;

        int p = right + 1 + tree_size;
        T result = e();

        do {
            --p;
            while (p > 1 && (p & 1)) p >>= 1;

            T next = op(tree[p], result);

            if (!predicate(next)) {
                while (p < tree_size) {
                    p = p << 1 | 1;
                    next = op(tree[p], result);

                    if (predicate(next)) {
                        result = next;
                        --p;
                    }
                }

                return p + 1 - tree_size;
            }

            result = next;
        } while ((p & -p) != p);

        return 0;
    }

    int size() const {
        return n;
    }

    bool empty() const {
        return n == 0;
    }
};

} // namespace zz_library

#endif // ZZ_LIBRARY_SEG_TREE_MONOID_TREE_HPP
