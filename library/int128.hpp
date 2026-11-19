#ifndef ZZ_LIBRARY_INT128_HPP
#define ZZ_LIBRARY_INT128_HPP

namespace zz_library {

using i128 = __int128_t;
using u128 = __uint128_t;

namespace detail {

inline bool parse_u128(const std::string& s, std::size_t pos, u128 limit, u128& res) {
    if (pos == s.size()) return false;

    u128 x = 0;
    for (; pos < s.size(); pos++) {
        if (s[pos] < '0' || s[pos] > '9') return false;

        int d = s[pos] - '0';
        if (x > (limit - d) / 10) return false;

        x = x * 10 + d;
    }

    res = x;
    return true;
}

}

}

inline std::ostream& operator<<(std::ostream& os, zz_library::u128 x) {
    if (x == 0) return os << '0';

    char s[40];
    std::size_t n = 0;

    while (x) {
        s[n++] = static_cast<char>('0' + x % 10);
        x /= 10;
    }

    while (n) os << s[--n];
    return os;
}

inline std::ostream& operator<<(std::ostream& os, zz_library::i128 x) {
    if (x < 0) {
        os << '-';
        return os << (zz_library::u128(0) - static_cast<zz_library::u128>(x));
    }

    return os << static_cast<zz_library::u128>(x);
}

inline std::istream& operator>>(std::istream& is, zz_library::u128& x) {
    std::string s;
    if (!(is >> s)) return is;

    if (!s.empty() && s[0] == '+') s.erase(0, 1);

    zz_library::u128 res;
    if (!zz_library::detail::parse_u128(s, 0, ~zz_library::u128(0), res)) {
        is.setstate(std::ios::failbit);
        return is;
    }

    x = res;
    return is;
}

inline std::istream& operator>>(std::istream& is, zz_library::i128& x) {
    std::string s;
    if (!(is >> s)) return is;

    bool neg = !s.empty() && s[0] == '-';
    std::size_t pos = (!s.empty() && (neg || s[0] == '+')) ? 1 : 0;

    constexpr zz_library::i128 MAX = ((zz_library::i128(1) << 126) - 1) * 2 + 1;
    constexpr zz_library::u128 LIMIT = zz_library::u128(1) << 127;

    zz_library::u128 res;
    if (!zz_library::detail::parse_u128(s, pos, neg ? LIMIT : MAX, res)) {
        is.setstate(std::ios::failbit);
        return is;
    }

    if (neg) {
        x = res == LIMIT ? -MAX - 1 : -static_cast<zz_library::i128>(res);
    }
    else {
        x = static_cast<zz_library::i128>(res);
    }

    return is;
}

#endif