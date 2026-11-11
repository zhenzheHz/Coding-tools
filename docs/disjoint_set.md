# Disjoint Set Union

## Include and select a type

Include `<bits/stdc++.h>` before the library header. The extensionless header
`<disjoint_set>` includes the complete implementation; `<disjoint_set.hpp>` is
also available.

```cpp
#include <bits/stdc++.h>
#include <disjoint_set>

int main() {
    zz_library::disjoint_set<int> dsu(5); // Elements 0 through 5.
    dsu.merge(1, 4);
    bool connected = dsu.check(1, 4);
}
```

The public type is `zz_library::disjoint_set<T, Type>`. `T` is a signed integral
index type and defaults to `int`. `Type` chooses the implementation and
defaults to `zz_library::basic`.

| Type | Features |
| --- | --- |
| `basic` | Union by size, path compression, component sizes, largest component, and member listing. |
| `almost` | Adds element moves, singleton resets, component sums, and member listing. |
| `rollback` | Supports snapshots and undo; omits path compression to make changes reversible. |

All implementations initialize elements in the closed range `[0, n]`, for a
total of `n + 1` elements. `size()` returns the number of components for
`basic` and `rollback`; for `almost`, it returns the number of elements. Use
`almost::components()` to get its component count. In debug mode, invalid
indices and invalid sizes are checked when compiled with `LOCAL` or
`ZZ_LIBRARY_DEBUG`.

## `basic` - standard DSU

The default type uses union by component size and path compression. Its
operations take amortized `O(α(n))`, except `group(x)`, which scans all elements
and takes `O(n α(n))`.

| Method | Description |
| --- | --- |
| `disjoint_set(n)` / `init(n)` | Initialize elements `[0, n]` as separate components. |
| `reset()` | Remove all elements. |
| `find(x)` | Return the representative of `x`'s component. |
| `check(x, y)` | Return whether `x` and `y` are connected. |
| `merge(x, y)` | Merge the components; return whether they were different. |
| `count(x)` | Return the size of `x`'s component. |
| `size()` | Return the number of components. |
| `max_size()` | Return the largest component size, or zero when empty. |
| `group(x)` | Return all elements in `x`'s component. |

`group` returns elements in ascending order because it scans indices from zero
upward.

## `almost` - DSU with element moves

This implementation supports moving an element to another component and
detaching an element into a singleton component. It tracks each component's
sum as well as its size. With union by size and path compression, `find`,
`check`, `merge`, `shift`, and `reset(x)` take amortized `O(α(N))`, where `N`
is the number of internal DSU nodes created so far. `group(x)` takes
`O(k log k)` for a component of `k` elements because its result is sorted.

| Method | Description |
| --- | --- |
| `disjoint_set(n)` / `init(n)` | Initialize elements `[0, n]` as separate components. |
| `reset()` | Remove all elements and internal nodes. |
| `size()` | Return the number of elements. |
| `components()` | Return the number of components. |
| `find(x)` | Return an element in `x`'s component (the current list head). |
| `check(x, y)` | Return whether `x` and `y` are connected. |
| `merge(x, y)` | Merge the components; return whether they were different. |
| `shift(x, y)` | Move `x` into `y`'s component; return whether a move occurred. |
| `reset(x)` | Detach `x` into a singleton component; return whether it changed. |
| `count(x)` | Return the size of `x`'s component. |
| `get_sum(x)` | Return the sum of element labels in `x`'s component. |
| `group(x)` | Return the sorted elements in `x`'s component. |

Each successful `shift` or `reset(x)` creates one internal node, so memory can
grow with the number of moves and resets. `get_sum` uses `int64_t` when `T` is
narrower than 64 bits and otherwise uses `T`.

## `rollback` - DSU with snapshots

This implementation uses union by size without path compression. `find`,
`check`, and `merge` take `O(log n)`; each successful merge adds one history
record. Failed merges do not add a record. Undoing `k` successful merges takes
`O(k)`.

```cpp
zz_library::disjoint_set<int, zz_library::rollback> dsu(5);
auto snap = dsu.snapshot();
dsu.merge(1, 4);
dsu.undo(snap); // Restore the state at snap.
```

| Method | Description |
| --- | --- |
| `disjoint_set(n)` / `init(n)` | Initialize elements `[0, n]` as separate components and clear history. |
| `reset()` | Remove all elements and history. |
| `find(x)` / `check(x, y)` | Find a representative / check connectivity. |
| `merge(x, y)` | Merge components and record a reversible change. |
| `count(x)` | Return the size of `x`'s component. |
| `size()` | Return the number of components. |
| `snapshot()` | Return the current history length. |
| `undo()` | Undo the most recent successful merge. |
| `undo(snapshot)` | Undo changes until the given snapshot is restored. |

Snapshots are history positions. They can only be used while their history is
still present; after undoing to an earlier position, a later branch may reuse
the same numeric snapshot value.
