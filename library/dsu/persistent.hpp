#ifndef ZZ_LIBRARY_DISJOINT_SET_PERSISTENT_HPP
#define ZZ_LIBRARY_DISJOINT_SET_PERSISTENT_HPP
#include "core.hpp"
namespace zz_library {
    template<class T>
    class disjoint_set<T, persistent, void> {
        struct Node {
            int left = 0, right = 0;
            T parent = 0, size = 1;
        };
        struct Value {
            T parent, size;
        };
        std::vector<Node> nodes { {
            }
        };
        std::vector<int> roots;
        std::size_t n = 0;
        int build(std::size_t l, std::size_t r) {
            int p = static_cast<int>(nodes.size());
            nodes.emplace_back();
            if (l == r) {
                nodes[static_cast<std::size_t>(p)].parent = static_cast<T>(l);
                return p;
            }
            std::size_t m = (l + r) / 2;
            int a = build(l, m), b = build(m + 1, r);
            nodes[static_cast<std::size_t>(p)].left = a;
            nodes[static_cast<std::size_t>(p)].right = b;
            return p;
        }
        Value get(int p, std::size_t l, std::size_t r, std::size_t x) const {
            if (l == r) return {
                nodes[static_cast<std::size_t>(p)].parent, nodes[static_cast<std::size_t>(p)].size
            };
            std::size_t m = (l + r) / 2;
            return x <= m ? get(nodes[static_cast<std::size_t>(p)].left, l, m, x) : get(nodes[static_cast<std::size_t>(p)].right, m + 1, r, x);
        }
        int set(int p, std::size_t l, std::size_t r, std::size_t x, Value v) {
            int q = static_cast<int>(nodes.size());
            nodes.push_back(nodes[static_cast<std::size_t>(p)]);
            if (l == r) {
                nodes[static_cast<std::size_t>(q)].parent = v.parent;
                nodes[static_cast<std::size_t>(q)].size = v.size;
                return q;
            }
            std::size_t m = (l + r) / 2;
            if (x <= m) nodes[static_cast<std::size_t>(q)].left = set(nodes[static_cast<std::size_t>(p)].left, l, m, x, v);
            else nodes[static_cast<std::size_t>(q)].right = set(nodes[static_cast<std::size_t>(p)].right, m + 1, r, x, v);
            return q;
        }
        T root(int version, T x) const {
            while (true) {
                Value v = get(roots[static_cast<std::size_t>(version)], 0, n - 1, static_cast<std::size_t>(x));
                if (v.parent == x) return x;
                x = v.parent;
            }
        }
    public:
        disjoint_set() = default;
        explicit disjoint_set(T m) {
            init(m);
        }
        void init(T m) {
            n = detail::dsu_index<T>::checked_size(m);
            nodes.clear();
            nodes.push_back({});
            roots.clear();
            roots.push_back(build(0, n - 1));
        }
        void reset() {
            nodes.assign(1, {});
            roots.clear();
            n = 0;
        }
        std::size_t versions() const {
            return roots.size();
        }
        T find(std::size_t version, T x) const {
            detail::dsu_index<T>::check(x, n);
            return root(static_cast<int>(version), x);
        }
        bool check(std::size_t version, T x, T y) const {
            return find(version, x) == find(version, y);
        }
        std::size_t merge(std::size_t version, T x, T y) {
            T a = root(static_cast<int>(version), x), b = root(static_cast<int>(version), y);
            int r = roots[version];
            if (a != b) {
                Value va = get(r, 0, n - 1, static_cast<std::size_t>(a)), vb = get(r, 0, n - 1, static_cast<std::size_t>(b));
                if (va.size < vb.size) {
                    std::swap(a, b);
                    std::swap(va, vb);
                }
                r = set(r, 0, n - 1, static_cast<std::size_t>(a), {a, static_cast<T>(va.size + vb.size)});
                r = set(r, 0, n - 1, static_cast<std::size_t>(b), {a, vb.size});
            }
            roots.push_back(r);
            return roots.size() - 1;
        }
    };
}
#endif
