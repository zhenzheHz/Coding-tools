# Heap

## Include and choose a heap

Include `<bits/stdc++.h>` before the library. The extensionless header `<heap>`
includes `heap.hpp`.

```cpp
#include <bits/stdc++.h>
#include <heap>

int main() {
    zz_library::heap<int> largest;
    largest.push(4);
    largest.push(9);
    int maximum = largest.top();
    largest.pop();

    zz_library::heap<int, zz_library::minimum> smallest;
    smallest.push(4);
    smallest.push(9);
    int minimum = smallest.top();
}
```

## Template parameters and namespace

The public type is `zz_library::heap<T, Type, Compare>`. `T` is the stored
value type; `Type` defaults to `zz_library::maximum`; `Compare` defaults to
`std::less<T>`. The class and all type tags are in `zz_library`.

## Type tags

`maximum` and `minimum` select binary heaps. The other type tags select the
corresponding heap structure:

| Type | Structure and behavior |
| --- | --- |
| `maximum` | Binary max-heap. `top()` returns the greatest element under `Compare`. |
| `minimum` | Binary min-heap. `top()` returns the least element under `Compare`. |
| `leftist` | Meldable max leftist heap. |
| `binomial` | Meldable max binomial heap. |
| `pairing` | Meldable max pairing heap. |
| `fibonacci` | Meldable max Fibonacci heap with decrease-key handles. |
| `interval` | Double-ended interval heap. |
| `SMMH` | Double-ended symmetric min-max heap. |

For the binary heaps, `Compare` determines the ordering: with the default
`std::less<T>`, `maximum` returns the numerically greatest value and `minimum`
returns the least. The other heap types also accept a comparator and use it to
order values; the single-ended heaps expose the greatest value under that
ordering. `Compare` must define a strict weak ordering.

## Shared conventions and comparator

For `maximum`, `leftist`, `binomial`, `pairing`, and `fibonacci`, `top()` returns
the greatest element under `Compare`. For `minimum`, it returns the least. The
double-ended heaps expose both ends and use the maximum end for `top()` and
`pop()`.

## Common methods

| Method | Description |
| --- | --- |
| `push(value)` / `emplace(args...)` | Add an element. Fibonacci heaps return a handle; the other types return `void`. |
| `top()` | Return the highest-priority element. For double-ended heaps, this is the maximum end. |
| `pop()` | Remove the element returned by `top()`. |
| `size()` / `empty()` | Return the number of elements / whether the heap is empty. |
| `clear()` | Remove all elements. |

Calling `top()` on an empty heap is invalid. `pop()` on an empty heap is also
invalid for the binary, leftist, binomial, pairing, and Fibonacci heaps.
`interval` and `SMMH` make `pop()` a no-op when empty; their `pop_min()` and
`pop_max()` are also no-ops when empty.

## Type-specific operations and complexity

| Type | Additional methods | Complexity |
| --- | --- | --- |
| `maximum`, `minimum` | — | `top`: `O(1)`; `push`, `pop`: `O(log n)`. |
| `leftist` | `merge(other)` | `top`: `O(1)`; `push`, `pop`, `merge`: `O(log n)`. |
| `binomial` | `merge(other)` | `top`, `push`, `pop`, `merge`: `O(log n)`. |
| `pairing` | `merge(other)` | `top`, `push`, `merge`: `O(1)`; `pop`: `O(log n)` amortized. |
| `fibonacci` | `merge(other)`, `decrease_key(handle, value)` | `top`, `push`, `merge`: `O(1)` amortized; `pop`: `O(log n)` amortized; `decrease_key`: `O(1)` amortized. |
| `interval`, `SMMH` | `min()`, `max()`, `top_min()`, `top_max()`, `pop_min()`, `pop_max()` | End access: `O(1)`; insertion and removal at either end: `O(log n)`. |

`merge(other)` transfers all elements from `other` and leaves it empty. It is
available for `leftist`, `binomial`, `pairing`, and `fibonacci`; use compatible
comparators for both heaps. These four pointer-based heap types are move-only.

For `interval` and `SMMH`, `min()` and `top_min()` return the least element;
`max()`, `top_max()`, and `top()` return the greatest. `pop_min()` and
`pop_max()` remove the corresponding end, while `pop()` removes the maximum.

For a Fibonacci heap, `push()` and `emplace()` return an opaque handle to the
inserted element. Pass a live handle from that heap to
`decrease_key(handle, new_value)`. The operation throws `std::invalid_argument`
if the new value has higher priority than the current value. Handles must refer
to live elements; do not use them after the corresponding element is removed
or the heap is cleared.
