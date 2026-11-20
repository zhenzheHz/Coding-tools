# Monotonic Stack Queries

## Include and construct

Include `<bits/stdc++.h>` before the library. The extensionless header
`<monotonic_stack>` includes `monotonic_stack.hpp`.

```cpp
#include <bits/stdc++.h>
#include <monotonic_stack>

int main() {
    zz_library::monotonic_stack<int> nearest({3, 1, 4, 2});
    int next_smaller = nearest[0].next_less(); // 1
}
```

## Template parameter and namespace

The public type is `zz_library::monotonic_stack<T>`. `T` is the value type and
must support the relational operators used by the queries. The class and its
nested query reference are in `zz_library`.

## Build and indexing conventions

The structure stores a snapshot of the input vector and precomputes all eight
nearest-index queries in `O(n)` time and `O(n)` additional memory. `init(values)`
replaces the snapshot and rebuilds the answers. Indices are zero-based. A query
returns `-1` if no matching element exists.

## Queries

Access a query reference with `stack[index]` or `stack.at(index)`. `operator[]`
requires a valid index; `at` throws `std::out_of_range` when the index is
invalid.

| Query | Meaning | Complexity |
| --- | --- | --- |
| `next_less()` | First index to the right with a strictly smaller value. | `O(1)` |
| `next_leq()` | First index to the right with a smaller or equal value. | `O(1)` |
| `next_greater()` | First index to the right with a strictly greater value. | `O(1)` |
| `next_geq()` | First index to the right with a greater or equal value. | `O(1)` |
| `prev_less()` | First index to the left with a strictly smaller value. | `O(1)` |
| `prev_leq()` | First index to the left with a smaller or equal value. | `O(1)` |
| `prev_greater()` | First index to the left with a strictly greater value. | `O(1)` |
| `prev_geq()` | First index to the left with a greater or equal value. | `O(1)` |
| `size()` / `empty()` | Return the snapshot length / whether it is empty. | `O(1)` |
