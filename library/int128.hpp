#ifndef ZZ_LIBRARY_INT128_HPP
#define ZZ_LIBRARY_INT128_HPP

namespace zz_library {

using i128 = __int128_t;
using u128 = __uint128_t;

}

std::ostream& operator<<(std::ostream& os, zz_library::u128 x) {
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

std::ostream& operator<<(std::ostream& os, zz_library::i128 x) {
    if (x < 0) {
        os << '-';
        return os << (zz_library::u128(0) - static_cast<zz_library::u128>(x));
    }

    return os << static_cast<zz_library::u128>(x);
}

#endif
