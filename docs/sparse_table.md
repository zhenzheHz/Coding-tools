# Sparse Table

## Include and select an operation

Include `<bits/stdc++.h>` before the library. The extensionless header
`<sparse_table>` includes `sparse_table.hpp`.

```cpp
#include <bits/stdc++.h>
#include <sparse_table>

int main() {
    std::vector<int> values{5, 2, 7, 3};
    zz_library::sparse_table<int, zz_library::RMQ> rmq(values);
    int minimum = rmq.query_min(1, 3);
    int maximum = rmq.query_max(1, 3);
}
```

## Template parameters and namespace

The public type is `zz_library::sparse_table<T, Type>`. `T` is the element
type; `Type` selects the compile-time operation. There is no default operation
tag. All tags and the class live in the `zz_library` namespace.

| Type tag | Query API | Requirements |
| --- | --- | --- |
| `RMQ` | `query_min(l, r)` and `query_max(l, r)` | `T` supports `<`. |
| `GCD` | `query(l, r)` | `T` is supported by `std::gcd`. |
| `AND` | `query(l, r)` | `T` supports bitwise `&`. |
| `OR` | `query(l, r)` | `T` supports bitwise `|`. |

## Shared conventions and complexity

All types support construction from `std::vector<T>`, `init(values)`, `size()`,
and `empty()`. Indices are zero-based and query intervals are closed `[l, r]`.
Queries require a non-empty valid range. In debug mode (`LOCAL` or
`ZZ_LIBRARY_DEBUG`), invalid ranges throw `std::out_of_range`.

| Operation | Complexity |
| --- | --- |
| Build / `init(values)` | `O(n log n)` time and memory. `RMQ` stores separate min and max tables. |
| Range query | `O(1)` |
| `size()` / `empty()` | `O(1)` |

Queries combine two overlapping blocks; this works for the supported idempotent
operations: minimum, maximum, GCD, bitwise AND, and bitwise OR. The structure is
static; call `init` to replace the sequence.

## Other operation tags

```cpp
zz_library::sparse_table<int, zz_library::GCD> gcd_table(values);
int divisor = gcd_table.query(0, 3);

zz_library::sparse_table<unsigned, zz_library::AND> and_table(bits);
unsigned common_bits = and_table.query(0, 3);
```
