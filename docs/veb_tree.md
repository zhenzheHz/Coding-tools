# van Emde Boas Tree

## Include and construct

Include `<bits/stdc++.h>` before the library. The extensionless header
`<veb_tree>` includes `veb_tree.hpp`.

```cpp
#include <bits/stdc++.h>
#include <veb_tree>

int main() {
    zz_library::veb_tree<int> values;
    values.insert(8);
    values.insert(3);

    int predecessor = values.predecessor(7).value_or(-1); // 3
    int successor = values.successor(7).value_or(-1);     // 8
    int smallest = values.min().value_or(-1);              // 3
    int largest = values.max().value_or(-1);               // 8
}
```

## Template parameter and namespace

The public type is `zz_library::veb_tree<T>`. `T` defaults to `int` and must be
a non-`bool` integral type no wider than 64 bits. The class is in the
`zz_library` namespace.

## Universe and complexity

The set stores unique non-negative values. For signed types, the universe is
`[0, max(T)]`; unsigned types use their full range. The implementation stores
only populated clusters in hash maps. Let `W` be the bit width of `T` and `n`
the number of stored values. Search operations take expected `O(log W)` time;
insertion and erasure take expected `O(W)` time because they may update both a
cluster and its summary. Storage is up to `O(n log W)`. Hash-map operations can
take linear time in the worst case.

## Methods

| Method | Description | Complexity |
| --- | --- | --- |
| `insert(value)` | Insert a value; return `true` if it was new. | Expected `O(W)` |
| `erase(value)` | Erase a value; return `true` if it existed. | Expected `O(W)` |
| `contains(value)` / `find(value)` | Test whether a value is present. | Expected `O(log W)` |
| `successor(value)` | Return the least stored value greater than or equal to `value`, or `std::nullopt` if none exists. | Expected `O(log W)` |
| `predecessor(value)` | Return the greatest stored value less than or equal to `value`, or `std::nullopt` if none exists. | Expected `O(log W)` |
| `min()` / `max()` | Return the smallest / largest value, or `std::nullopt` if the set is empty. | `O(1)` |
| `size()` / `empty()` | Return the number of values / whether the set is empty. | `O(1)` |
| `clear()` | Remove all values and allocated clusters. | `O(n log W)` |

Negative values passed to `insert`, `erase`, and `contains` are treated as
outside the universe. `successor` on a negative value returns the minimum;
`predecessor` returns `std::nullopt`. Results that may be absent use
`std::optional<T>`; use `.value_or(-1)` when `-1` is the desired sentinel.
