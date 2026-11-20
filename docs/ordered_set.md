# Ordered Set

## Include and construct

Include `<bits/stdc++.h>` before the library. The extensionless header
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

## Template parameter and namespace

The public type is `zz_library::ordered_set<T>`. `T` is the value type and
must be compatible with `std::less<T>`. Values equivalent under that ordering
are treated as duplicates. The class is in the `zz_library` namespace.

## Structure and complexity

The implementation is a randomized treap with subtree sizes. Search, insert,
erase, bounds, and order-statistic operations take expected `O(log n)` time;
the worst case is `O(n)`. `build(values)` sorts and deduplicates the input in
`O(n log n)` expected time. Iterators are bidirectional and visit elements in
ascending order.

## Methods

| Method | Description | Complexity |
| --- | --- | --- |
| `ordered_set()` / `ordered_set(values)` | Create an empty set or build from a vector, sorting and removing duplicates. | `O(n log n)` expected for values |
| `build(values)` | Replace contents from a vector, sorting and removing duplicates. | `O(n log n)` expected |
| `insert(value)` | Insert a value; return `{iterator, inserted}`. | Expected `O(log n)` |
| `erase(value)` | Remove a value; return `1` if it existed, otherwise `0`. | Expected `O(log n)` |
| `erase(iterator)` | Remove the pointed-to value and return the following iterator. | Expected `O(log n)` |
| `clear()` | Remove all values while keeping allocated storage available for reuse. | `O(n)` |
| `find(value)` / `contains(value)` | Find a value / test whether it exists. | Expected `O(log n)` |
| `count(value)` | Return `1` if present, otherwise `0`. | Expected `O(log n)` |
| `lower_bound(value)` / `upper_bound(value)` | Iterator to the first value not less than / greater than `value`. | Expected `O(log n)` |
| `find_by_order(k)` | Iterator to the zero-based k-th smallest value, or `end()` if `k >= size()`. | Expected `O(log n)` |
| `order_of_key(value)` | Number of stored values strictly less than `value`. | Expected `O(log n)` |
| `begin()` / `end()`; `rbegin()` / `rend()` | Bidirectional iterators for ascending / descending traversal. | `O(1)` to create; expected `O(log n)` per increment/decrement |
| `size()` / `empty()` | Return the number of unique values / whether the set is empty. | `O(1)` |
| `reserve(n)` / `capacity()` | Allocate storage for at least `n` nodes / report allocated node capacity. | `O(n)` / `O(1)` |

`find_by_order` uses zero-based ranks: `find_by_order(0)` is the minimum.
Missing searches and out-of-range `find_by_order` calls return `end()`;
dereferencing `end()` is invalid. Insertions and erasures preserve iterators to
elements that remain in the set. `clear`, `build`, assignment, and moving the
container invalidate iterators associated with replaced or moved elements.
