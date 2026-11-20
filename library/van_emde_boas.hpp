#ifndef ZZ_LIBRARY_VAN_EMDE_BOAS_HPP
#define ZZ_LIBRARY_VAN_EMDE_BOAS_HPP

namespace zz_library {

template<class T = int>
class veb_tree {
    static_assert(std::is_integral_v<T> && !std::is_same_v<T, bool>, "veb_tree requires a non-bool integer type");
    static_assert(sizeof(T) <= sizeof(std::uint64_t), "veb_tree supports integer types up to 64 bits");
    using key_type = std::uint64_t;
    static unsigned lowest_set_bit(std::uint64_t value) {
        unsigned bit = 0;
        while ((value & 1) == 0) { value >>= 1; ++bit; }
        return bit;
    }
    static unsigned highest_set_bit(std::uint64_t value) {
        unsigned bit = 0;
        while (value >>= 1) ++bit;
        return bit;
    }
    struct node {
        unsigned bits;
        unsigned lower_bits;
        unsigned upper_bits;
        key_type minimum = 0;
        key_type maximum = 0;
        std::size_t size = 0;
        std::uint64_t bitmap = 0;
        std::unique_ptr<node> summary;
        std::unordered_map<key_type, std::unique_ptr<node>> clusters;
        explicit node(unsigned bit_count)
            : bits(bit_count),
              lower_bits(bit_count <= 6 ? 0 : bit_count / 2),
              upper_bits(bit_count <= 6 ? 0 : bit_count - lower_bits) {}

        bool empty() const { return size == 0; }
        key_type high(key_type x) const { return x >> lower_bits; }
        key_type low(key_type x) const {
            return x & ((key_type(1) << lower_bits) - 1);
        }
        key_type index(key_type h, key_type l) const { return (h << lower_bits) | l; }

        bool contains(key_type x) const {
            if (empty() || x < minimum || x > maximum) return false;
            if (bits <= 6) return (bitmap & (std::uint64_t(1) << x)) != 0;
            if (x == minimum) return true;
            auto it = clusters.find(high(x));
            return it != clusters.end() && it->second->contains(low(x));
        }

        bool insert(key_type x) {
            if (contains(x)) return false;
            if (empty()) {
                minimum = maximum = x;
                size = 1;
                if (bits <= 6) bitmap |= std::uint64_t(1) << x;
                return true;
            }
            if (bits <= 6) {
                bitmap |= std::uint64_t(1) << x;
                if (x < minimum) minimum = x;
                if (x > maximum) maximum = x;
                ++size;
                return true;
            }
            if (x < minimum) std::swap(x, minimum);
            const key_type h = high(x), l = low(x);
            auto it = clusters.find(h);
            if (it == clusters.end()) {
                auto child = std::make_unique<node>(lower_bits);
                child->insert(l);
                if (!summary) summary = std::make_unique<node>(upper_bits);
                summary->insert(h);
                clusters.emplace(h, std::move(child));
            } else {
                it->second->insert(l);
            }
            if (x > maximum) maximum = x;
            ++size;
            return true;
        }

        bool erase(key_type x) {
            if (!contains(x)) return false;
            if (bits <= 6) {
                bitmap &= ~(std::uint64_t(1) << x);
                --size;
                if (size == 0) return true;
                minimum = static_cast<key_type>(lowest_set_bit(bitmap));
                maximum = highest_set_bit(bitmap);
                return true;
            }
            if (minimum == maximum) {
                size = 0;
                return true;
            }
            if (x == minimum) {
                const key_type first_cluster = summary->minimum;
                x = index(first_cluster, clusters.at(first_cluster)->minimum);
                minimum = x;
            }
            const key_type h = high(x), l = low(x);
            auto it = clusters.find(h);
            it->second->erase(l);
            if (it->second->empty()) {
                summary->erase(h);
                clusters.erase(it);
                if (summary->empty()) summary.reset();
            }
            if (x == maximum) {
                if (!summary) maximum = minimum;
                else {
                    const key_type last_cluster = summary->maximum;
                    maximum = index(last_cluster, clusters.at(last_cluster)->maximum);
                }
            }
            --size;
            return true;
        }

        std::optional<key_type> lower_bound(key_type x) const {
            if (empty() || x > maximum) return std::nullopt;
            if (x <= minimum) return minimum;
            if (bits <= 6) {
                const std::uint64_t candidates = bitmap & (~std::uint64_t(0) << x);
                if (!candidates) return std::nullopt;
                return static_cast<key_type>(lowest_set_bit(candidates));
            }
            const key_type h = high(x), l = low(x);
            auto it = clusters.find(h);
            if (it != clusters.end() && l <= it->second->maximum) {
                if (auto found = it->second->lower_bound(l)) return index(h, *found);
            }
            if (!summary) return std::nullopt;
            auto next_cluster = summary->lower_bound(h + 1);
            if (!next_cluster) return std::nullopt;
            return index(*next_cluster, clusters.at(*next_cluster)->minimum);
        }

        std::optional<key_type> predecessor(key_type x) const {
            if (empty() || x < minimum) return std::nullopt;
            if (x >= maximum) return maximum;
            if (bits <= 6) {
                const unsigned capped = static_cast<unsigned>(x);
                const std::uint64_t candidates = bitmap & ((std::uint64_t(1) << (capped + 1)) - 1);
                if (!candidates) return std::nullopt;
                return highest_set_bit(candidates);
            }
            const key_type h = high(x), l = low(x);
            auto it = clusters.find(h);
            if (it != clusters.end() && l >= it->second->minimum) {
                if (auto found = it->second->predecessor(l)) return index(h, *found);
            }
            if (!summary) return minimum;
            if (h == 0) return minimum;
            auto previous_cluster = summary->predecessor(h - 1);
            if (!previous_cluster) return minimum;
            return index(*previous_cluster, clusters.at(*previous_cluster)->maximum);
        }
    };

    static constexpr unsigned bit_width = std::numeric_limits<T>::digits;
    node root_{bit_width};

    static bool in_universe(T x) {
        if constexpr (std::is_signed_v<T>) return x >= 0;
        return true;
    }

    static key_type to_key(T x) { return static_cast<key_type>(x); }

public:
    using value_type = T;
    using size_type = std::size_t;

    veb_tree() = default;
    veb_tree(const veb_tree&) = delete;
    veb_tree& operator=(const veb_tree&) = delete;
    veb_tree(veb_tree&&) noexcept = default;
    veb_tree& operator=(veb_tree&&) noexcept = default;

    bool insert(T x) { return in_universe(x) && root_.insert(to_key(x)); }
    bool erase(T x) { return in_universe(x) && root_.erase(to_key(x)); }
    bool contains(T x) const { return in_universe(x) && root_.contains(to_key(x)); }
    bool find(T x) const { return contains(x); }

    std::optional<T> successor(T x) const {
        if constexpr (std::is_signed_v<T>) {
            if (x < 0) return min();
        }
        auto result = root_.lower_bound(to_key(x));
        if (!result) return std::nullopt;
        return static_cast<T>(*result);
    }
    std::optional<T> lower_bound(T x) const { return successor(x); }

    std::optional<T> predecessor(T x) const {
        if (!in_universe(x)) return std::nullopt;
        auto result = root_.predecessor(to_key(x));
        if (!result) return std::nullopt;
        return static_cast<T>(*result);
    }

    std::optional<T> min() const {
        if (root_.empty()) return std::nullopt;
        return static_cast<T>(root_.minimum);
    }
    std::optional<T> max() const {
        if (root_.empty()) return std::nullopt;
        return static_cast<T>(root_.maximum);
    }

    size_type size() const { return root_.size; }
    bool empty() const { return root_.empty(); }
    void clear() { root_ = node(bit_width); }
};

template<class T = std::uint64_t>
using van_emde_boas_set = veb_tree<T>;

}

#endif // ZZ_LIBRARY_VAN_EMDE_BOAS_HPP
