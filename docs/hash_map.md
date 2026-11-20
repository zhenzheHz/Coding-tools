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

## Template parameters and namespace

The public type is `zz_library::hash_map<K, V, Hash, KeyEqual>`. `K` and `V`
are the key and mapped types; `Hash` and `KeyEqual` default to the library's
hash and `std::equal_to<K>`. The type and supported hash helpers live in the
`zz_library` namespace.

## Shared behavior

The map uses open addressing, a randomized per-container seed, and a maximum
load factor of `0.6`. Lookup, insertion, and erasure take expected `O(1)` time
and worst-case `O(n)` time. Iteration order is unspecified.

The default hash supports integral and enum keys, `std::string`, and recursively
hashable `std::pair`, `std::vector`, `std::array`, `std::tuple`, ordered sets,
and unordered sets. For other key types, provide a `Hash` callable. It is
called as `hash(key)`; `KeyEqual` is called as
`equal(stored_key, key)`. Equal keys must produce equal hashes. Key and mapped
value types must be nothrow move-constructible.

## Methods

| Method | Description | Complexity |
| --- | --- | --- |
| `hash_map()` / `hash_map(hash, equal)` | Create an empty map with default or supplied hash and equality objects. | `O(1)` |
| `operator[](key)` | Return the mapped value, inserting a value-initialized one when absent. | Expected `O(1)` |
| `at(key)` | Return the mapped value; throw `std::out_of_range` if absent. | Expected `O(1)` |
| `find(key)` | Return an iterator to the entry or `end()`. | Expected `O(1)` |
| `contains(key)` / `count(key)` | Test whether a key exists; `count` returns zero or one. | Expected `O(1)` |
| `try_emplace(key, args...)` | Construct the mapped value only when absent; return `{iterator, inserted}`. | Expected `O(1)` |
| `insert(pair)` | Insert a key/value pair; return `{iterator, inserted}`. | Expected `O(1)` |
| `erase(key)` | Erase the key; return one if it existed, otherwise zero. | Expected `O(1)` |
| `erase(iterator)` | Erase the entry and return an iterator to the following slot. | Expected `O(1)` |
| `clear()` | Remove all entries while retaining the allocated table. | `O(capacity)` |
| `reserve(n)` | Reserve capacity for at least `n` entries. | `O(n)` expected |
| `begin()` / `end()` | Iterate over stored key/value pairs; iteration order is unspecified. | `begin()` may scan up to `O(capacity)` slots. |
| `size()` / `empty()` | Return the number of entries / whether the map is empty. | `O(1)` |
| `verify()` | Check table invariants; intended for debugging and validation. | Expected `O(capacity)` |

Iterators follow table slots and can be invalidated by insertion, erasure, or
rehashing. Do not retain iterators across modifications.
