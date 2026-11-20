# Trie

## Include and select a trie

Include `<bits/stdc++.h>` before the library. The extensionless header `<trie>`
includes `trie.hpp`.

```cpp
#include <bits/stdc++.h>
#include <trie>

int main() {
    zz_library::trie<> words;
    words.insert("apple");
    std::size_t matches = words.prefix_count("app");

    zz_library::binary_trie<unsigned> values;
    values.insert(5);
    values.insert(10);
    unsigned best_xor = values.max_xor(7);
}
```

## Types and namespace

The header provides two types in `zz_library`:

| Type | Stores | Duplicates |
| --- | --- | --- |
| `trie<T>` | Sequences of symbols of type `T`; defaults to `char`. | Supported and counted. |
| `binary_trie<T>` | Unsigned integer bit patterns; defaults to `unsigned int`. | Supported and counted. |

## `trie<T>` - sequence trie

For `T = char`, the class also accepts `std::string` and C-string arguments
directly. Other symbol types must be hashable by `std::unordered_map`. A
sequence of length `L` is traversed symbol by symbol; child-map operations are
expected `O(1)`, so sequence operations take expected `O(L)` time.

| Method | Description | Complexity |
| --- | --- | --- |
| `insert(sequence)` | Insert one occurrence of a vector, string, or C string for `char`. | Expected `O(L)` |
| `count(sequence)` / `contains(sequence)` | Count exact occurrences / test for at least one. | Expected `O(L)` |
| `prefix_count(prefix)` | Count stored sequences beginning with the prefix. | Expected `O(L)` |
| `erase(sequence)` | Erase one exact occurrence; return whether it existed. | Expected `O(L)` |
| `size()` / `empty()` | Return the number of stored sequences / whether the trie is empty. | `O(1)` |

The empty sequence is supported. Its exact count is the number of inserted
empty sequences; its prefix count is the total number of stored sequences.
Erasing a sequence decrements counts but does not reclaim unused trie nodes.

## `binary_trie<T>` - unsigned integer trie

`T` must be an unsigned integral type other than `bool`. Each value is treated
as a fixed-width bit string of `W` bits, where `W` is the width of `T`.

| Method | Description | Complexity |
| --- | --- | --- |
| `insert(value)` | Insert one occurrence. | `O(W)` |
| `erase(value)` | Erase one occurrence; return whether it existed. | `O(W)` |
| `count(value)` | Return the number of occurrences. | `O(W)` |
| `max_xor(mask)` / `min_xor(mask)` | Return the greatest / least XOR result `mask ^ value` among stored values. | `O(W)` |
| `count_xor_less(mask, limit)` | Count stored values for which `(mask ^ value) < limit`. | `O(W)` |
| `order_of_xor(mask, k)` | Return the zero-based k-th smallest XOR result, counting duplicates. | `O(W)` |
| `size()` / `empty()` | Return the number of stored values / whether the trie is empty. | `O(1)` |

The XOR queries return the XOR result, not the stored value. `max_xor` and
`min_xor` throw `std::out_of_range` on an empty trie. `order_of_xor` throws
when `k >= size()`. Erased paths remain allocated, so storage follows the
maximum trie size reached.
