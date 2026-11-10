# Fenwick Tree

## Include and construct

Include `<bits/stdc++.h>` before the library. The extensionless header `<fenwick>` includes `fenwick.hpp`.

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

The structure uses two Fenwick arrays to support range addition and range-sum queries. Indices are zero-based, and intervals are closed `[l, r]`.

## Methods

| Method | Description | Complexity |
| --- | --- | --- |
| `init(const std::vector<T>& values)` | Build from the initial sequence using a linear Fenwick build. | `O(n)` |
| `set(i, v)` | Assign `a[i] = v`. | `O(log n)` |
| `add(i, v)` | Add `v` to `a[i]`. | `O(log n)` |
| `update(l, r, x)` | Add `x` to every element in `[l, r]`. | `O(log n)` |
| `query(l, r)` | Return the sum of `[l, r]`. | `O(log n)` |

Valid ranges satisfy `0 <= l <= r < n`; point indices satisfy `0 <= i < n`. In debug mode, invalid indices and ranges throw standard exceptions. Define `LOCAL` or `ZZ_LIBRARY_DEBUG` before including the header to enable checks.

`set` reads the current point value, then applies the difference as a point range-add. The structure stores no separate array of point values. `T` must support addition, subtraction, multiplication by an index, and represent all intermediate sums without overflow. Space is `O(n)`.
