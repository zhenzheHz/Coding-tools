# Segment Trees: `magic_tree` and `seg_tree`

## Include and select a type

Include `<bits/stdc++.h>` before the library. The two umbrella headers `<magic_tree>` and `<seg_tree>` both include every segment-tree type. To keep a preprocessed submission small, include one implementation directly, for example `<seg_tree/zkw>` or `<seg_tree/beats>`.

```cpp
#include <bits/stdc++.h>
#include <seg_tree/zkw>

int main() {
    zz_library::Segment_tree<long long, zz_library::zkw> seg;
    seg.init({4, 2, 7, 2});
    seg.update(1, 3, 5);
    long long total = seg.query_sum(0, 3);
}
```

## Template parameters and namespace

The public type is `zz_library::Segment_tree<T, Type>`. `T` is the value and aggregate type (for example, `long long`); `Type` is a tag that chooses which implementation backs the tree. The second parameter is optional: `Segment_tree<T>` means `Segment_tree<T, zkw>`.

Both the class and its implementation tags live in the `zz_library` namespace. Either qualify both names, as in `zz_library::Segment_tree<long long, zz_library::lazy>`, or import the names you use with a `using` declaration. The include header must provide the selected tag; umbrella headers provide all tags, while a direct header such as `<seg_tree/zkw>` provides that implementation.

| Type tag | Implementation |
| --- | --- |
| `zkw` | Fixed-size sequence tree; the default. |
| `lazy` | Dynamic tree over `[0, 1,000,000,000]`. |
| `domain` | Dynamic tree over a configured coordinate interval. |
| `persistent` | Branchable versions of a sequence tree. |
| `kth` | Static order statistics and mex. |
| `tree2d` | Dynamic 2D point values. |
| `beats` | Segment Tree Beats. |

The tag is a compile-time choice, not a runtime setting. Each tag exposes its own operation set, so call only methods documented for the selected implementation.

## Shared conventions

- Sequence indices are zero-based; sequence intervals are closed `[l, r]`.
- `where_min` and `where_max` return the leftmost index on ties.
- `domain` and `tree2d` use configured, closed coordinate intervals.
- `lazy` covers indices `[0, 1,000,000,000]`.
- Define `LOCAL` (for example, compile with `-DLOCAL`) to enable debug checks. Without `LOCAL` or `ZZ_LIBRARY_DEBUG`, checks are compiled out.
- Select a numeric `T` that supports the required arithmetic and can hold range sums. `query_gcd` requires an integer-like type supporting `%`.

## `zkw` - fixed-size sequence

Initialize with `init(values)` or the vector constructor. Storage is contiguous; space is `O(n)`.

| Method | Description | Complexity |
| --- | --- | --- |
| `init(const std::vector<T>& values)` | Replace the sequence. | `O(n)` |
| `set(i, x)` | Assign one element. | `O(log n)` |
| `add(i, x)` | Add to one element. | `O(log n)` |
| `update(l, r, x)` | Add `x` to every value in `[l, r]`. | `O(log n)` |
| `assign(l, r, x)` | Assign every value in `[l, r]` to `x`. | `O(log n)` |
| `query_min(l, r)` / `query_max(l, r)` | Return a range extremum. | `O(log n)` |
| `where_min(l, r)` / `where_max(l, r)` | Return the leftmost extremum index. | `O(log n)` |
| `query_sum(l, r)` | Return the range sum. | `O(log n)` |
| `query_gcd(l, r)` | Return the non-negative GCD of all values. | `O(log n)` |
| `size()` / `empty()` | Return sequence length / whether it is empty. | `O(1)` |

`query_gcd` is over the values themselves. Empty sequences cannot be queried or updated.

## `lazy` - dynamic tree on a fixed coordinate range

The domain is `[0, 1,000,000,000]`; positions outside the initialized vector start at zero. `init(values)` maps `values[i]` to index `i`.

| Method | Description | Complexity |
| --- | --- | --- |
| `init(const std::vector<T>& values)` | Clear and initialize the prefix; remaining positions are zero. | `O(n + log U)` |
| `set(i, x)` / `add(i, x)` | Assign / increment one position. | `O(log U)` |
| `update(l, r, x)` | Add `x` to the range. | `O(log U)` |
| `assign(l, r, x)` | Assign the range. | `O(log U)` amortized |
| `query_min(l, r)` / `query_max(l, r)` | Return a range extremum. | `O(log U)` |
| `where_min(l, r)` / `where_max(l, r)` | Return the leftmost extremum index. | `O(log U)` |
| `query_sum(l, r)` / `query_gcd(l, r)` | Return range sum / value GCD. | `O(log U)` |
| `size()` / `empty()` | Return fixed domain size / `false`. | `O(1)` |

Here `U = 1,000,000,001` and the height is about 30. Range assignment can reclaim covered nodes; one call may reclaim `k` nodes, so the amortized bound accounts for node reclamation. Storage grows with the node-pool high-water mark.

## `domain` - dynamic weighted coordinate domain

Call `set_domain(l, r)` before other operations. Coordinates are signed 64-bit integers. The configured interval is closed, and resetting the domain clears all weights. Let `U = r - l + 1`.

| Method | Description | Complexity |
| --- | --- | --- |
| `set_domain(l, r)` | Set the domain and clear data. | `O(P)` |
| `set(x, v)` / `add(x, v)` | Assign / increment the weight at `x`. | `O(log U)` |
| `update(l, r, v)` | Add `v` to all weights in the interval. | `O(log U)` amortized |
| `assign(l, r, v)` | Assign all weights in the interval. | `O(log U)` amortized |
| `find_by_order(l, r, k)` | Find the k-th coordinate by weight, counting repetitions. | `O(log U)` |
| `find_by_rank(l, r, k)` | Find the k-th nonzero coordinate; each coordinate counts once. | `O(log U)` |
| `query_sum(l, r)` / `query_max(l, r)` | Return weight sum / maximum; implicit zeros count in the maximum. | `O(log U)` |
| `query_mark(l, r)` | Count coordinates with nonzero weight. | `O(log U)` |
| `lower_bound(l, r, x)` | First nonzero coordinate with weight at least `x`. | `O(log U)` |
| `upper_bound(l, r, x)` | First nonzero coordinate with weight strictly greater than `x`. | `O(log U)` |
| `count_less(l, r, x)` / `count_greater(l, r, x)` | Sum weights at coordinates below / above `x`. | `O(log U)` |
| `count(x)` | Return the weight at `x`. | `O(log U)` |
| `get_min()` / `get_max()` | Return the global smallest / largest nonzero coordinate. | `O(log U)` |

`P` is the old node-pool size when resetting the domain. Range updates can reclaim nodes and are amortized. Weights must remain non-negative. Negative increments are allowed only when the resulting weights stay non-negative. The order, rank, bound, and global-extremum methods return `std::optional<std::int64_t>`; check with `if (result)` and read the coordinate with `*result`.

## `persistent` - branchable versions

`init(values)` creates version `0`. Each update stores a new version and returns its `int` ID. An overload without a version uses the latest version; an overload with a version branches from that version. Old versions remain unchanged. Ranges are closed.

| Method | Description | Complexity |
| --- | --- | --- |
| `init(const std::vector<T>& values)` | Build version `0`. | `O(n)` |
| `update(l, r, x)` / `update(v, l, r, x)` | Range add from latest / version `v`; returns new ID. | `O(log n)` |
| `assign(l, r, x)` / `assign(v, l, r, x)` | Range assignment from latest / version `v`; returns new ID. | `O(log n)` |
| `add(i, x)` / `add(v, i, x)` | Point add from latest / version `v`; returns new ID. | `O(log n)` |
| `set(i, x)` / `set(v, i, x)` | Point assignment from latest / version `v`; returns new ID. | `O(log n)` |
| `query_sum(l, r)` / `query_min(l, r)` / `query_max(l, r)` | Query the latest version. | `O(log n)` |
| `query_sum(v, l, r)` / `query_min(v, l, r)` / `query_max(v, l, r)` | Query version `v`. | `O(log n)` |
| `latest_version()` / `version_count()` | Return latest ID / stored version count. | `O(1)` |
| `size()` / `empty()` | Return sequence length / whether it is empty. | `O(1)` |

Initial storage is `O(n)`; each update adds `O(log n)` nodes. Version IDs are reset by `init`.

## `kth` - static order statistics and mex

Initialize once; this type has no update methods. `k` starts at 1 and counts duplicate values. Let `sigma` be the number of distinct input values; the internal domain also includes mex candidates `[0, n]`.

| Method | Description | Complexity |
| --- | --- | --- |
| `init(const std::vector<T>& values)` | Build the static structure. | `O(n log(n + sigma))` |
| `find_kmin(l, r, k)` / `find_kmax(l, r, k)` | Return the k-th smallest / largest value. | `O(log(n + sigma))` |
| `count(l, r, a, b)` | Count values in the closed value interval `[a, b]`. | `O(log(n + sigma))` |
| `query_mex(l, r)` | Return the smallest missing non-negative integer. | `O(log(n + sigma))` |
| `query_min(l, r)` / `query_max(l, r)` | Return the range minimum / maximum. | `O(log(n + sigma))` |
| `query_sum(l, r)` | Return the range sum using prefix sums. | `O(1)` |
| `size()` / `empty()` | Return sequence length / whether it is empty. | `O(1)` |

K-th queries descend through the difference of two prefix roots; they do not binary-search the answer. `query_mex` requires integer-valued data. Choose `T` wide enough for the sum.

## `tree2d` - dynamic 2D point values

Set both closed signed 64-bit coordinate domains with `set_domain`, or set the axes separately. Any domain setter clears stored points. Let `X` and `Y` be the lengths of the two domains.

| Method | Description | Complexity |
| --- | --- | --- |
| `set_domain(x1, x2, y1, y2)` | Set both domains and clear data. | `O(S)` |
| `set_x_domain(l, r)` / `set_y_domain(l, r)` | Set one domain and clear data. | `O(S)` |
| `assign(x, y, v)` | Assign one point. | `O(log X log Y)` |
| `add(x, y, v)` | Increment one point. | `O(log X log Y)` |
| `query_sum(x1, x2, y1, y2)` | Sum a closed rectangle. | `O(log X log Y)` |
| `query_mark(x1, x2, y1, y2)` | Count nonzero points in a rectangle. | `O(log X log Y)` |
| `query_max(x1, x2, y1, y2)` | Rectangle maximum; empty cells count as zero. | `O(log X log Y)` |

`S` is the combined size of the existing node pools when resetting a domain. Queries require `x1 <= x2` and `y1 <= y2`.

## `beats` - Segment Tree Beats

This type tracks current extrema, not historical extrema. `second_min` and `second_max` mean the strict second distinct values.

| Method | Description | Complexity |
| --- | --- | --- |
| `init(const std::vector<T>& values)` | Build the sequence. | `O(n)` |
| `chmin(l, r, x)` / `chmax(l, r, x)` | Clamp range values from above / below. | Amortized; sequence-dependent |
| `update(l, r, x)` / `assign(l, r, x)` | Range add / range assignment. | `O(log n)` |
| `set(i, x)` / `add(i, x)` | Point assignment / point add. | `O(log n)` |
| `query_sum(l, r)` / `query_min(l, r)` / `query_max(l, r)` | Query range aggregates. | `O(log n)` |
| `min_count(l, r)` / `max_count(l, r)` | Count occurrences of the range minimum / maximum. | `O(log n)` |
| `second_min(l, r)` / `second_max(l, r)` | Return strict second extrema, or `std::nullopt`. | `O(log n)` |
| `size()` / `empty()` | Return sequence length / whether it is empty. | `O(1)` |

`chmin` and `chmax` are not guaranteed `O(log n)` per call; their complexity is amortized over an operation sequence. Storage is `O(n)`.

## Example: default and explicit tag

```cpp
#include <bits/stdc++.h>
#include <magic_tree>

int main() {
    // Omitting Type selects zkw.
    zz_library::Segment_tree<long long> fixed;
    fixed.init({3, 1, 4, 1});

    // The second template argument selects another implementation.
    zz_library::Segment_tree<long long, zz_library::lazy> dynamic;
    dynamic.init({3, 1, 4, 1});
    dynamic.update(1, 2, 5);
    long long answer = dynamic.query_sum(0, 3);
}
```
