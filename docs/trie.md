# Trie

## Include and choose a trie

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
    unsigned best = values.max_xor(7);
}
```

The header provides `zz_library::trie<T>` for sequences of symbols and
`zz_library::binary_trie<T>` for unsigned integer bit patterns. Both support
duplicates.

## Sequence trie

`trie<T>` stores vectors of symbols. For the default `T = char`, it also
accepts `std::string` and C-string arguments directly.

| Method | Description |
| --- | --- |
| `insert(sequence)` | Insert one occurrence of a vector, string, or C string (for `char`). |
| `count(sequence)` / `contains(sequence)` | Count exact occurrences / test for at least one. |
| `prefix_count(prefix)` | Count stored sequences beginning with the prefix. |
| `erase(sequence)` | Erase one exact occurrence; return whether it existed. |
| `size()` / `empty()` | Return the number of stored sequences / whether the trie is empty. |

The empty sequence is supported: its exact count and prefix count are the
number of inserted empty sequences and the total number of sequences,
respectively. Insert, count, and erase take `O(L)` expected time for sequence
length `L`, using hash maps for child lookup.

## Binary trie

`binary_trie<T>` requires an unsigned integral type other than `bool`; the
default is `unsigned int`. Each value is treated as a fixed-width bit string.

| Method | Description |
| --- | --- |
| `insert(value)` | Insert one occurrence. |
| `erase(value)` | Erase one occurrence; return whether it existed. |
| `count(value)` | Return the number of occurrences. |
| `max_xor(mask)` / `min_xor(mask)` | Return the greatest / least `mask ^ value` among stored values. |
| `count_xor_less(mask, limit)` | Count values for which `(mask ^ value) < limit`. |
| `order_of_xor(mask, k)` | Return the zero-based k-th smallest XOR result, counting duplicates. |
| `size()` / `empty()` | Return the number of stored values / whether the trie is empty. |

Operations take `O(W)` time, where `W` is the bit width of `T`. XOR extrema
queries throw `std::out_of_range` on an empty trie; `order_of_xor` also throws
when `k >= size()`.
