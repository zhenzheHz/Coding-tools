# Disjoint Set Union

## Include and select a type

Include `<bits/stdc++.h>` before the library. The extensionless header
`<disjoint_set>` includes the DSU variants.

```cpp
#include <bits/stdc++.h>
#include <disjoint_set>

int main() {
    zz_library::disjoint_set<int> dsu(5); // Elements 0 through 5.
    dsu.merge(1, 4);
    bool connected = dsu.check(1, 4);
}
```

## Template parameters and namespace

The public type is `zz_library::disjoint_set<T, Type, Group>`. `T` is a signed
integral index type and defaults to `int`. `Type` selects the implementation
and defaults to `zz_library::basic`. `Group` is used by the potential DSU; it
defaults to additive arithmetic when omitted. The type tags and DSU classes
are in `zz_library`.

| Type | Features |
| --- | --- |
| `basic` | Union by size, path compression, component sizes, largest component, and member listing. |
| `almost` | Element moves and singleton resets, component sums, and member listing. |
| `rollback` | Snapshots and undo; omits path compression so changes can be reversed. |
| `potential` | Maintains group-valued differences between connected elements. |
| `persistent` | Stores branchable versions; each merge creates a new version. |

## Shared conventions

All implementations initialize elements in the closed range `[0, n]`, for a
total of `n + 1` elements. `reset()` removes all state. Define `LOCAL` to
check invalid sizes and element indices; invalid inputs then throw standard
exceptions. Without `LOCAL`, these checks are omitted.

The `size()` method is variant-specific: it returns the number of components
for `basic`, `rollback`, and `potential`, but the number of elements for
`almost`. Use `almost::components()` for its component count. `persistent`
uses `versions()` to report its number of stored versions.

## `basic` - standard DSU

This is the default implementation. It uses union by component size and path
compression. `find`, `check`, `merge`, and component-size queries take
amortized `O(α(n))`. `group(x)` scans all elements and takes `O(n α(n))`.

| Method | Description | Complexity |
| --- | --- | --- |
| `disjoint_set(n)` / `init(n)` | Initialize elements `[0, n]` as separate components. | `O(n)` |
| `reset()` | Remove all elements. | `O(n)` |
| `find(x)` | Return the representative of `x`'s component. | Amortized `O(α(n))` |
| `check(x, y)` | Return whether `x` and `y` are connected. | Amortized `O(α(n))` |
| `merge(x, y)` | Merge the components; return whether they were different. | Amortized `O(α(n))` |
| `count(x)` / `count_of(x)` | Return the size of `x`'s component. | Amortized `O(α(n))` |
| `size()` | Return the number of components. | `O(1)` |
| `max_size()` | Return the largest component size, or zero when empty. | `O(1)` |
| `group(x)` | Return all elements in `x`'s component in ascending order. | `O(n α(n))` |

## `almost` - DSU with element moves

This variant can move an element to another component or detach it into a
singleton. It tracks each component's size and the sum of its element labels.
With union by size and path compression, `find`, `check`, `merge`, `shift`, and
`reset(x)` take amortized `O(α(N))`, where `N` is the number of internal DSU
nodes created so far. `group(x)` takes `O(n α(N))` and returns labels in
ascending order.

| Method | Description | Complexity |
| --- | --- | --- |
| `disjoint_set(n)` / `init(n)` | Initialize elements `[0, n]` as separate components. | `O(n)` |
| `reset()` | Remove all elements and internal nodes. | `O(N)` |
| `size()` / `components()` | Return the number of elements / components. | `O(1)` |
| `find(x)` | Return the current list head of `x`'s component. | Amortized `O(α(N))` |
| `check(x, y)` | Return whether `x` and `y` are connected. | Amortized `O(α(N))` |
| `merge(x, y)` | Merge the components; return whether they were different. | Amortized `O(α(N))` |
| `shift(x, y)` | Move `x` into `y`'s component; return whether a move occurred. | Amortized `O(α(N))` |
| `reset(x)` | Detach `x` into a singleton; return whether it changed. | Amortized `O(α(N))` |
| `count(x)` | Return the size of `x`'s component. | Amortized `O(α(N))` |
| `get_sum(x)` | Return the sum of labels in `x`'s component. | Amortized `O(α(N))` |
| `group(x)` | Return the sorted elements in `x`'s component. | `O(n α(N))` |

Each successful `shift` or `reset(x)` creates one internal node, so memory can
grow with the number of moves and resets. `get_sum` uses `int64_t` when `T` is
narrower than 64 bits and otherwise uses `T`.

## `rollback` - DSU with snapshots

This variant uses union by size without path compression. `find`, `check`, and
`merge` take `O(log n)`; each successful merge adds one history record. Failed
merges do not add a record. Undoing `k` successful merges takes `O(k)`.

```cpp
zz_library::disjoint_set<int, zz_library::rollback> dsu(5);
auto snap = dsu.snapshot();
dsu.merge(1, 4);
dsu.undo(snap); // Restore the state at snap.
```

| Method | Description | Complexity |
| --- | --- | --- |
| `disjoint_set(n)` / `init(n)` | Initialize elements `[0, n]` as separate components and clear history. | `O(n)` |
| `reset()` | Remove all elements and history. | `O(n)` |
| `find(x)` / `check(x, y)` | Find a representative / check connectivity. | `O(log n)` |
| `merge(x, y)` | Merge components and record a reversible change. | `O(log n)` |
| `count(x)` | Return the size of `x`'s component. | `O(log n)` |
| `size()` | Return the number of components. | `O(1)` |
| `snapshot()` | Return the current history length. | `O(1)` |
| `undo()` | Undo the most recent successful merge; return whether one was available. | `O(1)` |
| `undo(snapshot)` | Undo changes until the history reaches the given snapshot. | `O(k)` |

Snapshots are history positions. They can only be used while their history is
still present; after undoing to an earlier position, a later branch may reuse
the same numeric snapshot value.

## `potential` - DSU with group-valued differences

`disjoint_set<T, potential>` uses `additive_group<T>` by default, so
`merge(x, y, w)` constrains the potential difference `potential(y) -
potential(x)` to equal `w`. A custom `Group` can define another abelian group with `value_type`,
`identity()`, `op(a, b)`, and `inv(a)` operations.

| Method | Description | Complexity |
| --- | --- | --- |
| `disjoint_set(n)` / `init(n)` | Initialize elements `[0, n]` with zero relative potential. | `O(n)` |
| `reset()` | Remove all elements. | `O(n)` |
| `find(x)` / `check(x, y)` | Find a representative / test connectivity. | Amortized `O(α(n))` |
| `merge(x, y, w)` | Enforce the difference `w`; return whether the constraint is consistent. | Amortized `O(α(n))` |
| `diff(x, y)` | Return `potential(y) - potential(x)`, or `std::nullopt` if disconnected. | Amortized `O(α(n))` |
| `count(x)` | Return the size of `x`'s component. | Amortized `O(α(n))` |
| `size()` | Return the number of components. | `O(1)` |

If `x` and `y` are already connected, `merge` checks the requested relation
and returns `false` when it conflicts. Group operations must satisfy the
algebraic laws required for an additive group; for the default group, `T` must
support addition, negation, and equality comparison.

## `persistent` - branchable versions

`init(n)` creates version `0`. `merge(version, x, y)` creates and returns a new
version ID based on the selected version. Existing versions remain unchanged;
even a merge between already-connected elements creates a new version. The
implementation uses a persistent segment tree to store DSU parents and sizes.

| Method | Description | Complexity |
| --- | --- | --- |
| `disjoint_set(n)` / `init(n)` | Initialize elements `[0, n]` and create version `0`. | `O(n)` |
| `reset()` | Remove all elements and versions. | `O(n)` |
| `versions()` | Return the number of stored versions. | `O(1)` |
| `find(version, x)` / `check(version, x, y)` | Find a representative / test connectivity in a version. | `O(log² n)` |
| `merge(version, x, y)` | Create a new version after merging the components. | `O(log² n)` |

Each merge creates `O(log n)` persistent segment-tree nodes. Version IDs start
at zero and remain valid until `init` or `reset` replaces the structure.
