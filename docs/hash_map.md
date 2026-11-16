# Hash Map

## Include and construct

Include `<bits/stdc++.h>` before the library. The extensionless header
`<hash_map>` includes `hash_map.hpp`.

```cpp
#include <bits/stdc++.h>
#include <hash_map>

int main() {
    zz_library::hash_map<std::string, int> frequency;
    ++frequency["apple"];
    frequency.try_emplace("pear", 2);
    if (frequency.contains("apple")) {
        int count = frequency.at("apple");
    }
}
```

`zz_library::hash_map<K, V, Hash, KeyEqual>` stores unique keys and mapped
values. It uses open addressing with a randomized per-container seed and a
maximum load factor of 0.6. Lookup, insertion, and erasure take expected
`O(1)` time; the worst case is `O(n)`.

## Methods

| Method | Description |
| --- | --- |
| `hash_map()` / `hash_map(hash, equal)` | Create an empty map with default or supplied hash and equality objects. |
| `operator[](key)` | Return the mapped value, inserting a value-initialized one when the key is absent. |
| `at(key)` | Return the mapped value; throw `std::out_of_range` if absent. |
| `find(key)` | Return an iterator to the entry or `end()`. |
| `contains(key)` / `count(key)` | Test whether a key exists; `count` returns zero or one. |
| `try_emplace(key, args...)` | Construct the mapped value only when the key is absent; return `{iterator, inserted}`. |
| `insert(pair)` | Insert a key/value pair; return `{iterator, inserted}`. |
| `erase(key)` | Erase the key; return one if it existed, otherwise zero. |
| `erase(iterator)` | Erase the entry and return an iterator to the following slot. |
| `clear()` | Remove all entries while retaining the allocated table. |
| `reserve(n)` | Reserve capacity for at least `n` entries. |
| `begin()` / `end()` | Iterate over stored key/value pairs; iteration order is unspecified. |
| `size()` / `empty()` | Return the number of entries / whether the map is empty. |
| `verify()` | Check table invariants; useful for debug and validation. |

The default hash supports integral and enum keys; `std::string`; and recursively
hashable `std::pair`, `std::vector`, `std::array`, `std::tuple`, ordered sets,
and unordered sets. For other key types, provide a custom `Hash` callable. A
custom hash is called as `hash(key)`; `KeyEqual` is called as
`equal(stored_key, key)`. Equal keys must produce equal hashes.

The key and mapped value types must be nothrow move-constructible. Iterators
follow table slots and can be invalidated by insertion, erasure, or rehashing;
do not retain them across modifications.
