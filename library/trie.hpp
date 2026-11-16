#ifndef ZZ_LIBRARY_TRIE_HPP
#define ZZ_LIBRARY_TRIE_HPP

namespace zz_library {

template<class T = unsigned int>
class binary_trie {
    static_assert(std::is_integral_v<T> && std::is_unsigned_v<T> && !std::is_same_v<T, bool>,
                  "zz_library::binary_trie requires an unsigned integer type");

    static constexpr int bit_count = std::numeric_limits<T>::digits;
    static constexpr std::size_t none = static_cast<std::size_t>(-1);

    struct node {
        std::size_t child[2] = {none, none};
        std::size_t count = 0;
    };

    std::vector<node> nodes_ = std::vector<node>(1);

    bool contains(T value) const {
        std::size_t current = 0;
        for (int bit = bit_count - 1; bit >= 0; --bit) {
            const unsigned branch = static_cast<unsigned>((value >> bit) & T(1));
            const std::size_t next = nodes_[current].child[branch];
            if (next == none || nodes_[next].count == 0) return false;
            current = next;
        }
        return nodes_[current].count != 0;
    }

public:
    using value_type = T;
    using size_type = std::size_t;

    void insert(T value) {
        std::size_t current = 0;
        ++nodes_[current].count;
        for (int bit = bit_count - 1; bit >= 0; --bit) {
            const unsigned branch = static_cast<unsigned>((value >> bit) & T(1));
            std::size_t next = nodes_[current].child[branch];
            if (next == none) {
                next = nodes_.size();
                nodes_[current].child[branch] = next;
                nodes_.emplace_back();
            }
            current = next;
            ++nodes_[current].count;
        }
    }

    // Erases one occurrence and returns whether the value was present.
    bool erase(T value) {
        if (!contains(value)) return false;
        std::size_t current = 0;
        --nodes_[current].count;
        for (int bit = bit_count - 1; bit >= 0; --bit) {
            const unsigned branch = static_cast<unsigned>((value >> bit) & T(1));
            current = nodes_[current].child[branch];
            --nodes_[current].count;
        }
        return true;
    }

    size_type count(T value) const {
        std::size_t current = 0;
        for (int bit = bit_count - 1; bit >= 0; --bit) {
            const unsigned branch = static_cast<unsigned>((value >> bit) & T(1));
            const std::size_t next = nodes_[current].child[branch];
            if (next == none) return 0;
            current = next;
        }
        return nodes_[current].count;
    }

    size_type size() const { return nodes_[0].count; }
    bool empty() const { return size() == 0; }

    // Returns the maximum value of (mask ^ value) over all stored values.
    T max_xor(T mask) const {
        if (empty()) throw std::out_of_range("zz_library::binary_trie: query on empty trie");
        std::size_t current = 0;
        T result = 0;
        for (int bit = bit_count - 1; bit >= 0; --bit) {
            const unsigned mask_bit = static_cast<unsigned>((mask >> bit) & T(1));
            const unsigned preferred = mask_bit ^ 1U;
            std::size_t next = nodes_[current].child[preferred];
            unsigned chosen = preferred;
            if (next == none || nodes_[next].count == 0) {
                chosen ^= 1U;
                next = nodes_[current].child[chosen];
            }
            if (chosen != mask_bit) result |= T(1) << bit;
            current = next;
        }
        return result;
    }

    // Returns the minimum value of (mask ^ value) over all stored values.
    T min_xor(T mask) const {
        if (empty()) throw std::out_of_range("zz_library::binary_trie: query on empty trie");
        std::size_t current = 0;
        T result = 0;
        for (int bit = bit_count - 1; bit >= 0; --bit) {
            const unsigned mask_bit = static_cast<unsigned>((mask >> bit) & T(1));
            std::size_t next = nodes_[current].child[mask_bit];
            unsigned chosen = mask_bit;
            if (next == none || nodes_[next].count == 0) {
                chosen ^= 1U;
                next = nodes_[current].child[chosen];
            }
            if (chosen != mask_bit) result |= T(1) << bit;
            current = next;
        }
        return result;
    }

    // Counts stored values value for which (mask ^ value) < limit.
    size_type count_xor_less(T mask, T limit) const {
        size_type result = 0;
        std::size_t current = 0;
        for (int bit = bit_count - 1; bit >= 0 && current != none; --bit) {
            const unsigned mask_bit = static_cast<unsigned>((mask >> bit) & T(1));
            const unsigned limit_bit = static_cast<unsigned>((limit >> bit) & T(1));
            if (limit_bit) {
                const std::size_t same = nodes_[current].child[mask_bit];
                if (same != none) result += nodes_[same].count;
                current = nodes_[current].child[mask_bit ^ 1U];
            } else {
                current = nodes_[current].child[mask_bit];
            }
        }
        return result;
    }

    // Returns the zero-based k-th smallest (mask ^ value), counting duplicates.
    T order_of_xor(T mask, size_type k) const {
        if (k >= size()) throw std::out_of_range("zz_library::binary_trie: order is out of range");
        std::size_t current = 0;
        T result = 0;
        for (int bit = bit_count - 1; bit >= 0; --bit) {
            const unsigned mask_bit = static_cast<unsigned>((mask >> bit) & T(1));
            const std::size_t zero_branch = nodes_[current].child[mask_bit];
            const size_type zero_count = zero_branch == none ? 0 : nodes_[zero_branch].count;
            if (k < zero_count) {
                current = zero_branch;
            } else {
                k -= zero_count;
                result |= T(1) << bit;
                current = nodes_[current].child[mask_bit ^ 1U];
            }
        }
        return result;
    }
};

// A prefix tree over sequences of T. For trie<char>, string overloads are
// provided so it can be used directly as a string dictionary.
template<class T = char>
class trie {
    struct node {
        std::unordered_map<T, std::size_t> children;
        std::size_t prefix_count = 0;
        std::size_t terminal_count = 0;
    };

    std::vector<node> nodes_ = std::vector<node>(1);

    template<class Iterator>
    void insert_sequence(Iterator first, Iterator last) {
        std::size_t current = 0;
        ++nodes_[current].prefix_count;
        for (; first != last; ++first) {
            const T symbol = static_cast<T>(*first);
            auto it = nodes_[current].children.find(symbol);
            std::size_t next;
            if (it == nodes_[current].children.end()) {
                next = nodes_.size();
                nodes_[current].children.emplace(symbol, next);
                nodes_.emplace_back();
            } else {
                next = it->second;
            }
            current = next;
            ++nodes_[current].prefix_count;
        }
        ++nodes_[current].terminal_count;
    }

    template<class Iterator>
    std::size_t count_sequence(Iterator first, Iterator last, bool prefix) const {
        std::size_t current = 0;
        for (; first != last; ++first) {
            const T symbol = static_cast<T>(*first);
            auto it = nodes_[current].children.find(symbol);
            if (it == nodes_[current].children.end()) return 0;
            current = it->second;
        }
        return prefix ? nodes_[current].prefix_count : nodes_[current].terminal_count;
    }

    template<class Iterator>
    bool erase_sequence(Iterator first, Iterator last) {
        std::vector<std::size_t> path{0};
        std::size_t current = 0;
        for (; first != last; ++first) {
            const T symbol = static_cast<T>(*first);
            auto it = nodes_[current].children.find(symbol);
            if (it == nodes_[current].children.end()) return false;
            current = it->second;
            path.push_back(current);
        }
        if (nodes_[current].terminal_count == 0) return false;
        --nodes_[current].terminal_count;
        for (std::size_t index : path) --nodes_[index].prefix_count;
        return true;
    }

public:
    using value_type = T;
    using size_type = std::size_t;

    void insert(const std::vector<T>& sequence) {
        insert_sequence(sequence.begin(), sequence.end());
    }

    size_type count(const std::vector<T>& sequence) const {
        return count_sequence(sequence.begin(), sequence.end(), false);
    }

    bool contains(const std::vector<T>& sequence) const {
        return count(sequence) != 0;
    }

    size_type prefix_count(const std::vector<T>& prefix) const {
        return count_sequence(prefix.begin(), prefix.end(), true);
    }

    bool erase(const std::vector<T>& sequence) {
        return erase_sequence(sequence.begin(), sequence.end());
    }

    template<class U = T, std::enable_if_t<std::is_same_v<U, char>, int> = 0>
    void insert(const std::string& word) {
        insert_sequence(word.begin(), word.end());
    }

    template<class U = T, std::enable_if_t<std::is_same_v<U, char>, int> = 0>
    void insert(const char* word) {
        const std::string value(word);
        insert(value);
    }

    template<class U = T, std::enable_if_t<std::is_same_v<U, char>, int> = 0>
    size_type count(const std::string& word) const {
        return count_sequence(word.begin(), word.end(), false);
    }

    template<class U = T, std::enable_if_t<std::is_same_v<U, char>, int> = 0>
    size_type count(const char* word) const {
        const std::string value(word);
        return count(value);
    }

    template<class U = T, std::enable_if_t<std::is_same_v<U, char>, int> = 0>
    bool contains(const std::string& word) const {
        return count(word) != 0;
    }

    template<class U = T, std::enable_if_t<std::is_same_v<U, char>, int> = 0>
    bool contains(const char* word) const {
        return count(word) != 0;
    }

    template<class U = T, std::enable_if_t<std::is_same_v<U, char>, int> = 0>
    size_type prefix_count(const std::string& prefix) const {
        return count_sequence(prefix.begin(), prefix.end(), true);
    }

    template<class U = T, std::enable_if_t<std::is_same_v<U, char>, int> = 0>
    size_type prefix_count(const char* prefix) const {
        const std::string value(prefix);
        return prefix_count(value);
    }

    template<class U = T, std::enable_if_t<std::is_same_v<U, char>, int> = 0>
    bool erase(const std::string& word) {
        return erase_sequence(word.begin(), word.end());
    }

    template<class U = T, std::enable_if_t<std::is_same_v<U, char>, int> = 0>
    bool erase(const char* word) {
        const std::string value(word);
        return erase(value);
    }

    size_type size() const { return nodes_[0].prefix_count; }
    bool empty() const { return size() == 0; }
};

} // namespace zz_library

#endif // ZZ_LIBRARY_TRIE_HPP
