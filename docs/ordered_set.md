# Ordered Set

## Include and construct

Include `<bits/stdc++.h>` before the library header. The extensionless header
`<ordered_set>` includes `ordered_set.hpp`.

```cpp
#include <bits/stdc++.h>
#include <ordered_set>

int main() {
    zz_library::ordered_set<int> values{3, 1, 4, 1, 5};
    auto third = values.find_by_order(2); // 0-based: points to 3.
    std::size_t smaller = values.order_of_key(4); // 3 values are less than 4.
}
```

`zz_library::ordered_set<T>` stores unique values in sorted order. It uses
`std::less<T>` for ordering, so `T` must be compatible with that comparator.
Values equivalent under the comparator are treated as duplicates.

The implementation is a randomized treap with subtree sizes. Search, insert,
erase, bounds, and order-statistic operations take expected `O(log n)` time;
the worst case is `O(n)`. `build(values)` sorts and deduplicates its input in
`O(n log n)` expected time. Iterators are bidirectional and visit elements in
ascending order.

## Methods

| Method | Description |
| --- | --- |
| `ordered_set()` / `ordered_set(values)` | Create an empty set or build from a vector, sorting and removing duplicates. |
| `build(values)` | Replace contents from a vector, sorting and removing duplicates. |
| `insert(value)` | Insert a value; returns `{iterator, inserted}`. |
| `erase(value)` | Remove a value; returns `1` if it existed, otherwise `0`. |
| `erase(iterator)` | Remove the pointed-to value and return the following iterator. |
| `clear()` | Remove all values while keeping allocated storage available for reuse. |
| `find(value)` / `contains(value)` | Find a value / test whether it exists. |
| `count(value)` | Return `1` if present, otherwise `0`. |
| `lower_bound(value)` | Iterator to the first value not less than `value`. |
| `upper_bound(value)` | Iterator to the first value greater than `value`. |
| `find_by_order(k)` | Iterator to the zero-based k-th smallest value, or `end()` if `k >= size()`. |
| `order_of_key(value)` | Number of stored values strictly less than `value`. |
| `begin()` / `end()` | Bidirectional iterators for ascending traversal. |
| `rbegin()` / `rend()` | Reverse iterators for descending traversal. |
| `size()` / `empty()` | Return the number of unique values / whether the set is empty. |
| `reserve(n)` / `capacity()` | Allocate storage for at least `n` nodes / report allocated node capacity. |

`find_by_order` uses zero-based ranks: `find_by_order(0)` is the minimum.
Missing value searches and out-of-range `find_by_order` calls return `end()`.
As with standard containers, dereferencing `end()` is invalid. Insertion and
erasure preserve iterators to elements that remain in the set. `clear`,
`build`, assignment, and moving the container invalidate iterators associated
with the elements being replaced or moved.
