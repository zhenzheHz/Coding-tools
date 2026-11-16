# Monotonic Stack Queries

## Include and build

Include `<bits/stdc++.h>` before the library. The extensionless header
`<monotonic_stack>` includes `monotonic_stack.hpp`.

```cpp
#include <bits/stdc++.h>
#include <monotonic_stack>

int main() {
    zz_library::monotonic_stack<int> next({3, 1, 4, 2});
    int index = next[0].next_less(); // 1
}
```

The class takes a snapshot of a vector and precomputes the nearest qualifying
index on each side of every element. Construction takes `O(n)` time and
`O(n)` memory. Call `init(values)` to replace the sequence and rebuild all
queries.

## Queries

Indexing is zero-based. Each query returns the nearest index satisfying its
relation, or `-1` when no such index exists.

| Query | Meaning |
| --- | --- |
| `next_less()` | First index to the right with a strictly smaller value. |
| `next_leq()` | First index to the right with a smaller or equal value. |
| `next_greater()` | First index to the right with a strictly greater value. |
| `next_geq()` | First index to the right with a greater or equal value. |
| `prev_less()` | First index to the left with a strictly smaller value. |
| `prev_leq()` | First index to the left with a smaller or equal value. |
| `prev_greater()` | First index to the left with a strictly greater value. |
| `prev_geq()` | First index to the left with a greater or equal value. |

Use `stack[index]` or the bounds-checking `stack.at(index)` to get a lightweight
query reference. `at` throws `std::out_of_range` for an invalid index;
`operator[]` requires a valid index. `size()` and `empty()` report the stored
sequence size. Values must support the relational operators used by the
queries.
