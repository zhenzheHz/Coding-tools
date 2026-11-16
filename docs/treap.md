# Treap

## Include and choose a mode

Include `<bits/stdc++.h>` before the library. The extensionless header
`<treap>` includes `treap.hpp` (and the 128-bit helper used by its aggregates).

```cpp
#include <bits/stdc++.h>
#include <treap>

int main() {
    zz_library::treap<int> sequence({4, 1, 3});
    sequence.reverse(0, 2);

    zz_library::treap<int, zz_library::ordered> ordered;
    ordered.insert(7);
    ordered.insert(2);
    auto smallest = ordered.find_by_order(0);
}
```

`zz_library::treap<T>` is an implicit treap that treats values as a mutable
sequence. `zz_library::treap<T, zz_library::ordered>` is an ordered multiset
treap. Both use randomized priorities and provide expected `O(log n)` updates
and queries; worst-case time is `O(n)`.

## Implicit sequence treap

Positions and interval endpoints are zero-based; intervals are closed `[l, r]`.

| Method | Description |
| --- | --- |
| `init(values)` | Replace the sequence from a vector. |
| `at(i)` / `operator[](i)` / `set(i, x)` | Read or replace one element. |
| `insert(i, x)` / `erase(i)` | Insert before position `i` / erase the element at `i`. |
| `erase_interval(l, r)` | Erase a closed interval. |
| `push_front(x)` / `push_back(x)`; `pop_front()` / `pop_back()` | Sequence-end operations. |
| `front()` / `back()` | Read the first / last value. |
| `reverse(l, r)` | Reverse a closed interval. |
| `rotate(l, r, m)` | Rotate `[l, r]` so `[l, m]` follows `[m+1, r]`. |
| `splice(l, r, p)` | Remove `[l, r]` and insert it before position `p` in the original sequence. |
| `affine(l, r, a, b)` | Apply `x = a*x + b` to values in the interval. |
| `query_sum(l, r)` | Return the interval sum using `sum_type`. |
| `query_min(l, r)` / `query_max(l, r)` | Return interval extrema when `T` supports `<`. |
| `to_vector()` / `for_each(f)` | Export or visit values in sequence order. |
| `size()` / `empty()` / `clear()` / `reserve(n)` | Inspect or manage the sequence. |

`affine` requires a value type that supports the operation. `set_mod(m)` enables
modular arithmetic for integral types up to 64 bits; set it once before adding
elements. In modular mode, values must be in `[0, m)` when `LOCAL` checks are
enabled, and minimum/maximum queries are unavailable. `set_seed(seed)` can be
used once before insertion to make the randomized structure reproducible.

## Ordered multiset treap

The ordered mode requires `T` to support `operator<`. Duplicate values are
stored and counted. It provides `init(values)`, `insert(x)`, `erase_one(x)`,
`erase_all(x)`, `find(x)`, `contains(x)`, `count(x)`, `lower_bound(x)`,
`upper_bound(x)`, `find_by_order(k)`, `order_of_key(x)`, `front()`, `back()`,
`begin()` / `end()`, `size()`, `empty()`, `clear()`, and `reserve(n)`.
`find_by_order` uses a zero-based rank and returns `end()` when the rank is out
of range. `order_of_key(x)` counts elements strictly less than `x`.

Both modes expose bidirectional iterators. Structural updates invalidate
iterators; with `LOCAL`, dereferencing an invalidated iterator is checked.
