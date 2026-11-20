# Treap

## Include and choose a mode

Include `<bits/stdc++.h>` before the library. The extensionless header
`<treap>` includes `treap.hpp` and the 128-bit helper used by its aggregates.

```cpp
#include <bits/stdc++.h>
#include <treap>

int main() {
    zz_library::treap<int> sequence({4, 1, 3});
    sequence.reverse(0, 2);

    zz_library::treap<int, zz_library::ordered> ordered;
    ordered.insert(7);
    ordered.insert(2);
    int smallest = *ordered.find_by_order(0);
}
```

## Template parameters and namespace

The public type is `zz_library::treap<T, Mode>`. `T` is the stored value type;
`Mode` defaults to `zz_library::implicit` and can be set to
`zz_library::ordered`. The class and mode tags are in `zz_library`.

| Mode | Behavior |
| --- | --- |
| `implicit` | Mutable sequence with positional and range operations. |
| `ordered` | Ordered multiset; duplicate values are retained and counted. |

## Shared conventions and complexity

Both modes use randomized priorities. Operations that follow or modify a tree
path take expected `O(log n)` time and can take `O(n)` in the worst case. Set
`set_seed(seed)` once before inserting values if deterministic priorities are
needed. With `LOCAL`, invalid positions and intervals are checked, and
invalidated iterators are detected.

## `implicit` - sequence treap

Positions and interval endpoints are zero-based; intervals are closed `[l, r]`.

| Method | Description | Complexity |
| --- | --- | --- |
| `init(values)` | Replace the sequence from a vector. | `O(n)` |
| `at(i)` / `operator[](i)` | Read one element. | Expected `O(log n)` |
| `set(i, x)` | Replace one element. | Expected `O(log n)` |
| `insert(i, x)` / `erase(i)` | Insert before position `i` / erase the element at `i`. | Expected `O(log n)` |
| `erase_interval(l, r)` | Erase a closed interval. | Expected `O(log n + k)` for `k` erased elements |
| `push_front(x)` / `push_back(x)`; `pop_front()` / `pop_back()` | Insert or remove at either end. | Expected `O(log n)` |
| `front()` / `back()` | Read the first / last value. | Expected `O(log n)` |
| `reverse(l, r)` | Reverse a closed interval. | Expected `O(log n)` |
| `rotate(l, r, m)` | Rotate `[l, r]` so `[l, m]` follows `[m+1, r]`. | Expected `O(log n)` |
| `splice(l, r, p)` | Remove `[l, r]` and insert it before position `p` in the original sequence. | Expected `O(log n)` |
| `affine(l, r, a, b)` | Apply `x = a*x + b` to values in the interval. | Expected `O(log n)` |
| `query_sum(l, r)` | Return the interval sum using `sum_type`. | Expected `O(log n)` |
| `query_min(l, r)` / `query_max(l, r)` | Return interval extrema when `T` supports `<`. | Expected `O(log n)` |
| `to_vector()` / `for_each(f)` | Export or visit values in sequence order. | `O(n)` |
| `begin()` / `end()` | Bidirectional iterators in sequence order. | Expected `O(1)` to create; dereference `O(log n)` |
| `size()` / `empty()` | Return sequence length / whether it is empty. | `O(1)` |
| `clear()` / `reserve(n)` | Clear values / reserve node storage. | `O(n)` / `O(n)` |

`affine` requires a value type that supports the operation. `query_sum` returns
`sum_type`, which widens built-in integer types where needed. `set_mod(m)` enables modular arithmetic for integral types up to 64 bits; call
it once before adding elements. `set_seed(seed)` is also a one-time, pre-insertion
setting. `rotate(l, r, m)` moves `[m+1, r]` before `[l, m]`. With `LOCAL`, inserted values must be in `[0, m)`. Minimum and maximum
queries are unavailable in modular mode.

## `ordered` - ordered multiset treap

The ordered mode requires `T` to support `operator<`. Its iterators visit values
in ascending order, including duplicates.

| Method | Description | Complexity |
| --- | --- | --- |
| `init(values)` | Replace contents from a vector. | Expected `O(n log n)` |
| `insert(x)` | Insert one occurrence. | Expected `O(log n)` |
| `erase_one(x)` / `erase_all(x)` | Remove one / all occurrences; return whether/how many were removed. | Expected `O(log n)` |
| `find(x)` / `contains(x)` / `count(x)` | Find an occurrence / test membership / count duplicates. | Expected `O(log n)` |
| `lower_bound(x)` / `upper_bound(x)` | Iterator to first value not less than / greater than `x`. | Expected `O(log n)` |
| `find_by_order(k)` | Iterator to zero-based k-th smallest value, or `end()` if out of range. | Expected `O(log n)` |
| `order_of_key(x)` | Count elements strictly less than `x`. | Expected `O(log n)` |
| `front()` / `back()` | Return the minimum / maximum; throw on an empty tree. | Expected `O(log n)` |
| `begin()` / `end()` | Bidirectional iterators in sorted order. | Expected `O(1)` to create; dereference `O(log n)` |
| `size()` / `empty()` / `clear()` / `reserve(n)` | Inspect or manage the multiset. | `O(1)` / `O(1)` / `O(n)` / `O(n)` |

`find_by_order` uses a zero-based rank and returns `end()` when the rank is out
of range. `order_of_key(x)` counts duplicates strictly below `x`. Structural
updates invalidate iterators; with `LOCAL`, dereferencing an invalidated
iterator is checked.
