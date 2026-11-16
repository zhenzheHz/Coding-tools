# Heap

## Include and select a heap

Include `<bits/stdc++.h>` before the library. The extensionless header `<heap>`
includes `heap.hpp`. Select a type with the second template argument:

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

The public type is `zz_library::heap<T, Type, Compare>`. `Type` defaults to
`zz_library::maximum`; `Compare` defaults to `std::less<T>`.

| Type | Behavior |
| --- | --- |
| `maximum` | Binary max-heap; `top()` returns the greatest value under `Compare`. |
| `minimum` | Binary min-heap; `top()` returns the least value under `Compare`. |
| `leftist_tree` | Meldable max leftist heap. |
| `double_ended` | Ordered multiset heap with access to both ends. |
| `f_heap` | Fibonacci heap with a minimum root and decrease-key handles. |

## Common methods

| Method | Description |
| --- | --- |
| `push(value)` / `emplace(args...)` | Add an element. `f_heap` returns a handle to the inserted element. |
| `top()` | Return the root element (maximum for the default type; minimum for `f_heap`). |
| `pop()` | Remove the root element. |
| `size()` / `empty()` | Return the number of elements / whether the heap is empty. |
| `clear()` | Remove all elements. |

Calling `top()` or `pop()` on an empty heap is invalid. The binary heaps provide
`O(1)` `top` and `O(log n)` `push`/`pop`. `leftist_tree` supports expected
`O(log n)` meld and `O(log n)` `push`/`pop`. `double_ended` supports `O(1)` end
access and `O(log n)` insertion/removal. Fibonacci heap `push` and `top` are
`O(1)` amortized; `pop` is `O(log n)` amortized and `decrease_key` is `O(1)`
amortized.

## Type-specific operations

`leftist_tree` and `f_heap` provide `merge(other)`, which moves all elements
from `other` into the current heap and leaves `other` empty. These types are
move-only. `double_ended` provides `min()` / `top_min()`, `max()` / `top_max()`,
`pop_min()`, and `pop_max()`; its `top()` and `pop()` use the maximum end.

For `f_heap`, pass a handle returned by `push` or `emplace` to
`decrease_key(handle, new_value)`. The operation throws `std::invalid_argument`
if `new_value` is greater than the current value. Handles are opaque and must
refer to a live element in that heap.
