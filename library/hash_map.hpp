#ifndef ZZ_LIBRARY_HASH_MAP_HPP
#define ZZ_LIBRARY_HASH_MAP_HPP

namespace zz_library {

namespace detail {

inline std::uint64_t splitmix64(std::uint64_t x) {
    x += 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}

inline std::uint64_t hash_seed() {
    static const std::uint64_t seed = [] {
        std::uint64_t x = std::chrono::steady_clock::now().time_since_epoch().count();
        try {
            std::random_device rd;
            x ^= static_cast<std::uint64_t>(rd()) << 32;
            x ^= static_cast<std::uint64_t>(rd());
        } catch (...) {}
        static int anchor;
        x ^= reinterpret_cast<std::uintptr_t>(&anchor);
        return splitmix64(x);
    }();
    return seed;
}

template<class T>
struct dependent_false : std::false_type {};

template<class T, class = void>
struct has_std_hash : std::false_type {};

template<class T>
struct has_std_hash<T, std::void_t<decltype(std::hash<T>{}(std::declval<const T&>()))>> : std::true_type {};

template<class T>
std::uint64_t hash_element(const T& x, std::uint64_t seed);

inline std::uint64_t hash_sequence_begin(std::uint64_t seed, std::size_t size) {
    return splitmix64(seed ^ static_cast<std::uint64_t>(size));
}

inline std::uint64_t hash_append(std::uint64_t h, std::uint64_t x) {
    return splitmix64(h + x);
}

template<class Tuple, std::size_t... I>
std::uint64_t hash_tuple(const Tuple& t, std::uint64_t seed, std::index_sequence<I...>) {
    std::uint64_t h = splitmix64(seed ^ sizeof...(I));
    ((h = hash_append(h, hash_element(std::get<I>(t), seed))), ...);
    return h;
}

}

template<class K, class Enable = void>
struct default_hash {
    std::uint64_t operator()(const K& key, std::uint64_t seed) const {
        if constexpr (detail::has_std_hash<K>::value) {
            return detail::splitmix64(static_cast<std::uint64_t>(std::hash<K>{}(key)) ^ seed);
        } else {
            static_assert(detail::dependent_false<K>::value, "No default hash available. Please provide a custom Hash.");
            return 0;
        }
    }
};

template<class K>
struct default_hash<K, std::enable_if_t<std::is_integral_v<K>>> {
    std::uint64_t operator()(K key, std::uint64_t seed) const {
        return detail::splitmix64(static_cast<std::uint64_t>(key) ^ seed);
    }
};

template<class K>
struct default_hash<K, std::enable_if_t<std::is_enum_v<K>>> {
    std::uint64_t operator()(K key, std::uint64_t seed) const {
        using U = std::underlying_type_t<K>;
        return detail::splitmix64(static_cast<std::uint64_t>(static_cast<U>(key)) ^ seed);
    }
};

template<>
struct default_hash<std::string> {
    std::uint64_t operator()(const std::string& s, std::uint64_t seed) const {
        std::uint64_t h = detail::hash_sequence_begin(seed, s.size());
        std::size_t i = 0;
        for (; i + 8 <= s.size(); i += 8) {
            std::uint64_t x = 0;
            std::memcpy(&x, s.data() + i, 8);
            h = detail::hash_append(h, x);
        }
        if (i < s.size()) {
            std::uint64_t x = 0;
            std::memcpy(&x, s.data() + i, s.size() - i);
            h = detail::hash_append(h, x);
        }
        return h;
    }
};

template<class A, class B>
struct default_hash<std::pair<A, B>> {
    std::uint64_t operator()(const std::pair<A, B>& p, std::uint64_t seed) const {
        std::uint64_t h = detail::hash_sequence_begin(seed, 2);
        h = detail::hash_append(h, detail::hash_element(p.first, seed));
        h = detail::hash_append(h, detail::hash_element(p.second, seed));
        return h;
    }
};

template<class T>
struct default_hash<std::vector<T>> {
    std::uint64_t operator()(const std::vector<T>& v, std::uint64_t seed) const {
        std::uint64_t h = detail::hash_sequence_begin(seed, v.size());
        for (const auto& x : v) h = detail::hash_append(h, detail::hash_element(static_cast<T>(x), seed));
        return h;
    }
};

template<class T, std::size_t N>
struct default_hash<std::array<T, N>> {
    std::uint64_t operator()(const std::array<T, N>& a, std::uint64_t seed) const {
        std::uint64_t h = detail::hash_sequence_begin(seed, N);
        for (const auto& x : a) h = detail::hash_append(h, detail::hash_element(x, seed));
        return h;
    }
};

template<class... T>
struct default_hash<std::tuple<T...>> {
    std::uint64_t operator()(const std::tuple<T...>& t, std::uint64_t seed) const {
        return detail::hash_tuple(t, seed, std::index_sequence_for<T...>{});
    }
};

template<class T, class C, class A>
struct default_hash<std::set<T, C, A>> {
    std::uint64_t operator()(const std::set<T, C, A>& s, std::uint64_t seed) const {
        std::uint64_t h = detail::hash_sequence_begin(seed, s.size());
        for (const auto& x : s) h = detail::hash_append(h, detail::hash_element(x, seed));
        return h;
    }
};

template<class T, class C, class A>
struct default_hash<std::multiset<T, C, A>> {
    std::uint64_t operator()(const std::multiset<T, C, A>& s, std::uint64_t seed) const {
        std::uint64_t h = detail::hash_sequence_begin(seed, s.size());
        for (const auto& x : s) h = detail::hash_append(h, detail::hash_element(x, seed));
        return h;
    }
};

template<class T, class H, class E, class A>
struct default_hash<std::unordered_set<T, H, E, A>> {
    std::uint64_t operator()(const std::unordered_set<T, H, E, A>& s, std::uint64_t seed) const {
        std::uint64_t sum = 0, sum2 = 0;
        for (const auto& x : s) {
            std::uint64_t h = detail::splitmix64(detail::hash_element(x, seed) ^ seed);
            sum += h;
            sum2 += h * h;
        }
        return detail::splitmix64(seed ^ sum ^ detail::splitmix64(sum2) ^ detail::splitmix64(s.size()));
    }
};

template<class T, class H, class E, class A>
struct default_hash<std::unordered_multiset<T, H, E, A>> {
    std::uint64_t operator()(const std::unordered_multiset<T, H, E, A>& s, std::uint64_t seed) const {
        std::uint64_t sum = 0, sum2 = 0;
        for (const auto& x : s) {
            std::uint64_t h = detail::splitmix64(detail::hash_element(x, seed) ^ seed);
            sum += h;
            sum2 += h * h;
        }
        return detail::splitmix64(seed ^ sum ^ detail::splitmix64(sum2) ^ detail::splitmix64(s.size()));
    }
};

namespace detail {

template<class T>
std::uint64_t hash_element(const T& x, std::uint64_t seed) {
    if constexpr (std::is_integral_v<T>) return static_cast<std::uint64_t>(x);
    else if constexpr (std::is_enum_v<T>) {
        using U = std::underlying_type_t<T>;
        return static_cast<std::uint64_t>(static_cast<U>(x));
    } else return default_hash<T>{}(x, seed);
}

}

template<class K, class V, class Hash = default_hash<K>, class KeyEqual = std::equal_to<K>>
class hash_map {
    static_assert(std::is_nothrow_move_constructible_v<K>, "hash_map requires nothrow move constructible Key.");
    static_assert(std::is_nothrow_move_constructible_v<V>, "hash_map requires nothrow move constructible Value.");
    static_assert(std::is_nothrow_swappable_v<Hash>, "hash_map requires nothrow swappable Hash.");
    static_assert(std::is_nothrow_swappable_v<KeyEqual>, "hash_map requires nothrow swappable KeyEqual.");

    using value_type_impl = std::pair<K, V>;
    using storage_type = std::aligned_storage_t<sizeof(value_type_impl), alignof(value_type_impl)>;

    struct Slot {
        storage_type storage;
        std::uint64_t hash = 0;
        std::uint8_t ctrl = 0x80;
    };

    static constexpr std::uint8_t EMPTY = 0x80;
    static constexpr std::uint8_t DELETED = 0xFE;
    static constexpr std::size_t MIN_CAPACITY = 16;

    Hash hash_;
    KeyEqual equal_;
    std::uint64_t seed_ = detail::hash_seed();
    std::unique_ptr<Slot[]> slots_;
    std::size_t capacity_ = 0;
    std::size_t size_ = 0;
    std::size_t growth_left_ = 0;

    static std::size_t threshold(std::size_t cap) { return cap / 5 * 3 + cap % 5 * 3 / 5; }

    static std::size_t normalize_capacity(std::size_t n) {
        std::size_t cap = MIN_CAPACITY;
        while (threshold(cap) < n) {
            if (cap > std::numeric_limits<std::size_t>::max() / 2) throw std::length_error("hash_map capacity overflow");
            cap <<= 1;
        }
        return cap;
    }

    value_type_impl* value(std::size_t i) {
        return std::launder(reinterpret_cast<value_type_impl*>(&slots_[i].storage));
    }

    const value_type_impl* value(std::size_t i) const {
        return std::launder(reinterpret_cast<const value_type_impl*>(&slots_[i].storage));
    }

    std::uint64_t get_hash(const K& key) const {
        if constexpr (std::is_same_v<Hash, default_hash<K>>) return hash_(key, seed_);
        else return detail::splitmix64(static_cast<std::uint64_t>(hash_(key)) ^ seed_);
    }

    std::size_t find_index(const K& key, std::uint64_t h) const {
        if (!capacity_) return 0;
        std::size_t i = h & (capacity_ - 1);
        std::uint8_t h2 = static_cast<std::uint8_t>(h >> 57);
        for (std::size_t step = 0; step < capacity_; ++step, i = (i + 1) & (capacity_ - 1)) {
            std::uint8_t c = slots_[i].ctrl;
            if (c == EMPTY) return capacity_;
            if (c == h2 && slots_[i].hash == h && equal_(value(i)->first, key)) return i;
        }
        return capacity_;
    }

    std::pair<std::size_t, bool> locate_insert(const K& key, std::uint64_t h) const {
        std::size_t i = h & (capacity_ - 1), first_deleted = capacity_;
        std::uint8_t h2 = static_cast<std::uint8_t>(h >> 57);
        for (std::size_t step = 0; step < capacity_; ++step, i = (i + 1) & (capacity_ - 1)) {
            std::uint8_t c = slots_[i].ctrl;
            if (c == EMPTY) return {first_deleted == capacity_ ? i : first_deleted, false};
            if (c == DELETED) {
                if (first_deleted == capacity_) first_deleted = i;
            } else if (c == h2 && slots_[i].hash == h && equal_(value(i)->first, key)) return {i, true};
        }
        return {first_deleted, false};
    }

    void rehash(std::size_t new_capacity) {
        std::unique_ptr<Slot[]> next(new Slot[new_capacity]);
        for (std::size_t i = 0; i < capacity_; ++i) if (slots_[i].ctrl < EMPTY) {
            std::size_t p = slots_[i].hash & (new_capacity - 1);
            while (next[p].ctrl != EMPTY) p = (p + 1) & (new_capacity - 1);
            new (&next[p].storage) value_type_impl(std::move(*value(i)));
            next[p].hash = slots_[i].hash;
            next[p].ctrl = slots_[i].ctrl;
            value(i)->~value_type_impl();
        }
        slots_ = std::move(next);
        capacity_ = new_capacity;
        growth_left_ = threshold(capacity_) - size_;
    }

    void ensure_capacity() {
        if (!capacity_) rehash(MIN_CAPACITY);
    }

    template<class KeyArg, class... Args>
    std::pair<std::size_t, bool> emplace_impl(KeyArg&& key, Args&&... args) {
        ensure_capacity();
        std::uint64_t h = get_hash(key);
        auto [p, found] = locate_insert(key, h);
        if (found) return {p, false};
        if (p == capacity_ || (slots_[p].ctrl == EMPTY && growth_left_ == 0)) {
            if (size_ < threshold(capacity_)) rehash(capacity_);
            else {
                if (capacity_ > std::numeric_limits<std::size_t>::max() / 2) throw std::length_error("hash_map capacity overflow");
                rehash(capacity_ * 2);
            }
            std::tie(p, found) = locate_insert(key, h);
            if (found) return {p, false};
        }
        bool was_empty = slots_[p].ctrl == EMPTY;
        new (&slots_[p].storage) value_type_impl(std::piecewise_construct, std::forward_as_tuple(std::forward<KeyArg>(key)), std::forward_as_tuple(std::forward<Args>(args)...));
        slots_[p].hash = h;
        slots_[p].ctrl = static_cast<std::uint8_t>(h >> 57);
        ++size_;
        if (was_empty) --growth_left_;
        return {p, true};
    }

    void destroy_all() noexcept {
        for (std::size_t i = 0; i < capacity_; ++i) if (slots_[i].ctrl < EMPTY) value(i)->~value_type_impl();
    }

public:
    using key_type = K;
    using mapped_type = V;
    using value_type = std::pair<K, V>;
    using size_type = std::size_t;

    template<bool IsConst>
    class basic_iterator {
        friend class hash_map;
        template<bool> friend class basic_iterator;
        using map_type = std::conditional_t<IsConst, const hash_map, hash_map>;
        map_type* map_ = nullptr;
        std::size_t index_ = 0;
        basic_iterator(map_type* map, std::size_t index) : map_(map), index_(index) { skip(); }
        void skip() {
            if (!map_) return;
            while (index_ < map_->capacity_ && map_->slots_[index_].ctrl >= EMPTY) ++index_;
        }
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = std::pair<K, V>;
        using reference = std::conditional_t<IsConst, const value_type&, value_type&>;
        using pointer = std::conditional_t<IsConst, const value_type*, value_type*>;
        basic_iterator() = default;
        template<bool B = IsConst, std::enable_if_t<B, int> = 0>
        basic_iterator(const basic_iterator<false>& other) : map_(other.map_), index_(other.index_) {}
        reference operator*() const { return *map_->value(index_); }
        pointer operator->() const { return map_->value(index_); }
        basic_iterator& operator++() { ++index_; skip(); return *this; }
        basic_iterator operator++(decltype(0)) { auto t = *this; ++*this; return t; }
        template<bool B>
        bool operator==(const basic_iterator<B>& other) const { return map_ == other.map_ && index_ == other.index_; }
        template<bool B>
        bool operator!=(const basic_iterator<B>& other) const { return !(*this == other); }
    };

    using iterator = basic_iterator<false>;
    using const_iterator = basic_iterator<true>;

    hash_map() : hash_(), equal_() { rehash(MIN_CAPACITY); }
    explicit hash_map(const Hash& hash, const KeyEqual& equal = KeyEqual()) : hash_(hash), equal_(equal) { rehash(MIN_CAPACITY); }

    hash_map(const hash_map& other) : hash_(other.hash_), equal_(other.equal_), seed_(other.seed_) {
        if (!other.capacity_) return;
        rehash(other.capacity_);
        try {
            for (const auto& x : other) try_emplace(x.first, x.second);
        } catch (...) {
            destroy_all();
            throw;
        }
    }

    hash_map(hash_map&& other) noexcept(std::is_nothrow_move_constructible_v<Hash> && std::is_nothrow_move_constructible_v<KeyEqual>)
        : hash_(std::move(other.hash_)), equal_(std::move(other.equal_)), seed_(other.seed_), slots_(std::move(other.slots_)), capacity_(other.capacity_), size_(other.size_), growth_left_(other.growth_left_) {
        other.capacity_ = other.size_ = other.growth_left_ = 0;
    }

    hash_map& operator=(const hash_map& other) {
        if (this == &other) return *this;
        hash_map temp(other);
        swap(temp);
        return *this;
    }

    hash_map& operator=(hash_map&& other) noexcept(std::is_nothrow_move_constructible_v<Hash> && std::is_nothrow_move_constructible_v<KeyEqual> && std::is_nothrow_swappable_v<Hash> && std::is_nothrow_swappable_v<KeyEqual>) {
        if (this == &other) return *this;
        hash_map temp(std::move(other));
        swap(temp);
        return *this;
    }

    ~hash_map() { destroy_all(); }

    void swap(hash_map& other) noexcept(std::is_nothrow_swappable_v<Hash> && std::is_nothrow_swappable_v<KeyEqual>) {
        using std::swap;
        swap(hash_, other.hash_);
        swap(equal_, other.equal_);
        swap(seed_, other.seed_);
        swap(slots_, other.slots_);
        swap(capacity_, other.capacity_);
        swap(size_, other.size_);
        swap(growth_left_, other.growth_left_);
    }

    std::size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }
    iterator begin() { return iterator(this, 0); }
    iterator end() { return iterator(this, capacity_); }
    const_iterator begin() const { return const_iterator(this, 0); }
    const_iterator end() const { return const_iterator(this, capacity_); }
    const_iterator cbegin() const { return begin(); }
    const_iterator cend() const { return end(); }

    iterator find(const K& key) {
        std::size_t i = find_index(key, get_hash(key));
        return iterator(this, i);
    }
    const_iterator find(const K& key) const {
        std::size_t i = find_index(key, get_hash(key));
        return const_iterator(this, i);
    }
    bool contains(const K& key) const { return find_index(key, get_hash(key)) != capacity_; }
    std::size_t count(const K& key) const { return contains(key); }

    template<class... Args>
    std::pair<iterator, bool> try_emplace(const K& key, Args&&... args) {
        auto [i, inserted] = emplace_impl(key, std::forward<Args>(args)...);
        return {iterator(this, i), inserted};
    }
    template<class... Args>
    std::pair<iterator, bool> try_emplace(K&& key, Args&&... args) {
        auto [i, inserted] = emplace_impl(std::move(key), std::forward<Args>(args)...);
        return {iterator(this, i), inserted};
    }
    std::pair<iterator, bool> insert(const value_type& x) { return try_emplace(x.first, x.second); }
    std::pair<iterator, bool> insert(value_type&& x) { return try_emplace(std::move(x.first), std::move(x.second)); }

    V& operator[](const K& key) { return try_emplace(key).first->second; }
    V& operator[](K&& key) { return try_emplace(std::move(key)).first->second; }

    V& at(const K& key) {
        auto it = find(key);
        if (it == end()) throw std::out_of_range("hash_map::at");
        return it->second;
    }
    const V& at(const K& key) const {
        auto it = find(key);
        if (it == end()) throw std::out_of_range("hash_map::at");
        return it->second;
    }

    std::size_t erase(const K& key) {
        std::size_t i = find_index(key, get_hash(key));
        if (i == capacity_) return 0;
        value(i)->~value_type_impl();
        --size_;
        std::size_t next = (i + 1) & (capacity_ - 1);
        if (slots_[next].ctrl == EMPTY) {
            slots_[i].ctrl = EMPTY;
            ++growth_left_;
        } else slots_[i].ctrl = DELETED;
        return 1;
    }

    iterator erase(const_iterator pos) {
        std::size_t i = pos.index_;
        if (pos.map_ != this || i >= capacity_) return end();
        std::size_t next_index = i + 1;
        value(i)->~value_type_impl();
        --size_;
        std::size_t next = (i + 1) & (capacity_ - 1);
        if (slots_[next].ctrl == EMPTY) {
            slots_[i].ctrl = EMPTY;
            ++growth_left_;
        } else slots_[i].ctrl = DELETED;
        return iterator(this, next_index);
    }

    void clear() {
        destroy_all();
        for (std::size_t i = 0; i < capacity_; ++i) slots_[i].ctrl = EMPTY;
        size_ = 0;
        growth_left_ = threshold(capacity_);
    }

    void reserve(std::size_t n) {
        std::size_t need = normalize_capacity(n);
        if (need > capacity_) rehash(need);
    }

#ifdef ZZ_LIBRARY_DEBUG
    bool verify() const {
        if (!capacity_) return size_ == 0 && !slots_;
        if ((capacity_ & (capacity_ - 1)) || capacity_ < MIN_CAPACITY) return false;
        std::size_t full = 0, deleted = 0;
        for (std::size_t i = 0; i < capacity_; ++i) {
            if (slots_[i].ctrl < EMPTY) {
                ++full;
                if (slots_[i].ctrl != static_cast<std::uint8_t>(slots_[i].hash >> 57)) return false;
                if (find_index(value(i)->first, slots_[i].hash) != i) return false;
            } else if (slots_[i].ctrl == DELETED) ++deleted;
            else if (slots_[i].ctrl != EMPTY) return false;
        }
        return full == size_ && size_ + deleted <= threshold(capacity_) && growth_left_ == threshold(capacity_) - size_ - deleted;
    }
#endif
};

}

#endif
