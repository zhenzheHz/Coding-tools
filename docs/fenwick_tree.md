# Fenwick Tree

## Include and construct

Include `<bits/stdc++.h>` before the library. The extensionless header
`<fenwick>` includes `fenwick.hpp`.

```cpp
#include <bits/stdc++.h>
#include <fenwick>

int main() {
    zz_library::fenwick_tree<long long> fw;
    fw.init({2, 5, 1, 4});
    fw.update(1, 3, 2);
    long long answer = fw.query(0, 3);
}
```

## Template parameter and namespace

The public type is `zz_library::fenwick_tree<T>`. `T` is the stored value and
sum type. The class is in the `zz_library` namespace.

## Shared conventions

- Indices are zero-based and ranges are closed `[l, r]`.
- The structure uses two Fenwick arrays to support range addition and range-sum queries.
- Valid ranges satisfy `0 <= l <= r < n`; point indices satisfy `0 <= i < n`.
- Define `LOCAL` or `ZZ_LIBRARY_DEBUG` before including the header to enable bounds checks. Invalid indices and ranges throw `std::out_of_range` in debug mode.
- `T` must support addition, subtraction, unary negation, and multiplication by a `T`-convertible index. It must hold intermediate sums without overflow.

## Methods

| Method | Description | Complexity |
| --- | --- | --- |
| `init(const std::vector<T>& values)` | Build from the initial sequence using a linear Fenwick build. | `O(n)` |
| `set(i, value)` | Assign `a[i] = value`. | `O(log n)` |
| `add(i, value)` | Add `value` to `a[i]`. | `O(log n)` |
| `update(l, r, value)` | Add `value` to every element in `[l, r]`. | `O(log n)` |
| `query(l, r)` | Return the sum of `[l, r]`. | `O(log n)` |

`set` reads the current point value, then applies the difference as a point
range-add. The structure stores no separate array of point values. Storage is
`O(n)`.
