# Sparse Table

## Include and select an operation

Include `<bits/stdc++.h>` before the library header. The extensionless header
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

The public type is `zz_library::sparse_table<T, Type>`. Choose one operation
tag explicitly:

| Type | Query |
| --- | --- |
| `RMQ` | `query_min(l, r)` and `query_max(l, r)` |
| `GCD` | `query(l, r)` for the range GCD |
| `AND` | `query(l, r)` for bitwise AND |
| `OR` | `query(l, r)` for bitwise OR |

All tags support construction from a `std::vector<T>`, `init(values)`,
`size()`, and `empty()`. `RMQ` stores separate min and max tables; the other
tags store one table.

## Range and complexity

Indices are zero-based and query intervals are closed `[l, r]`. The structure
is static: build it once, then answer queries in `O(1)`. Initialization takes
`O(n log n)` time and memory (`RMQ` stores two tables, so its constant factor is
larger). Empty ranges and out-of-bounds indices are rejected in debug mode when
compiled with `LOCAL` or `ZZ_LIBRARY_DEBUG`; queries require a non-empty range.

Sparse-table queries combine two overlapping blocks. This is valid for the
provided idempotent operations (minimum, maximum, GCD, bitwise AND, and bitwise
OR). The class does not provide point updates; call `init` to replace the
entire sequence.

## Other operation tags

```cpp
zz_library::sparse_table<int, zz_library::GCD> gcd_table(values);
int divisor = gcd_table.query(0, 3);

zz_library::sparse_table<unsigned, zz_library::AND> and_table(bits);
unsigned common_bits = and_table.query(0, 3);
```

`GCD` requires an integer type supported by `std::gcd`. `AND` and `OR` require
types that support the corresponding bitwise operators.
