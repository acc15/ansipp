#pragma once

#include <cstdint>
#include <type_traits>
#include <concepts>
#include <cstring>
#include <limits>
#include <utility>
#include <bit>

// by default only decimals are optimized
#ifndef ANSIPP_FAST_INTEGRAL
#define ANSIPP_FAST_INTEGRAL 0x04
#endif

namespace ansipp {

constexpr char to_digit(unsigned int v, bool upper = false) { 
    return static_cast<char>((v < 10 ? '0' : upper ? 'A' - 10 : 'a' - 10) + v); 
}

template <typename T>
constexpr T cpow(T base, unsigned int pow) {
    if (base == 1 || pow == 0) return 1;
    if (base == 0) return 0;
    
    T result = 1;
    while (pow > 0) {
        if (pow % 2 == 0) { 
            pow /= 2;
            base *= base; 
        } else {
            --pow;
            result *= base;
        }
    }
    return result;
}

template <typename T>
constexpr std::pair<unsigned int, T> cmaxpow(unsigned int base) {
    unsigned int pow = 0;
    T v = 1;
    for (T nv; nv = static_cast<T>(v * base), static_cast<T>(nv / base) == v; ++pow, v = nv);
    return std::pair<unsigned int, T>(pow, v);
}

template <unsigned int base, typename T = std::uintmax_t>
struct pow_table {
    constexpr static unsigned int size = cmaxpow<T>(base).first;
    struct table_data {
        T pow[size];
        constexpr table_data() {
            T v = base;
            for (unsigned int i = 0; i < size; ++i) {
                pow[i] = v;
                v *= base;
            }
        }
    };
    constexpr static table_data data = {};
};

template <unsigned int base, unsigned int digits, bool upper = false>
struct integral_lookup {
    constexpr static unsigned int pow = cpow(base, digits);
    struct table_data {
        char chars[pow][digits];
        constexpr table_data() {
            for (unsigned int v = 0; v < pow; ++v) {
                unsigned int n = v;
                for (unsigned int c = digits; c-- > 0; n /= base) {
                    chars[v][c] = to_digit(n % base, upper);
                }
            }
        }
    };
    constexpr static table_data data = {};
};

constexpr std::uintmax_t integral_abs(std::intmax_t v) {
    const auto uv = static_cast<std::uintmax_t>(v);
    if (v >= 0) return uv;
#ifdef _MSC_VER
#pragma warning(suppress: 4146)
    return -uv;
#else
    return -uv;
#endif
}

constexpr unsigned int unsigned_integral_length(std::uintmax_t value, unsigned int base) {
    if (value < base) return 1;
#if (ANSIPP_FAST_INTEGRAL & 0x04) != 0 
    if (base == 10) [[ likely ]] {
        // idea from http://www.graphics.stanford.edu/~seander/bithacks.html
        // requires pow_table<10> = sizeof(std::uintmax_t)*(std::numeric_limits<std::uintmax_t>::digits10 + 1)
        // if uintmax_t is 64 bit then 8*19 = 152 bytes
        unsigned int approx_log10 = static_cast<unsigned int>(std::bit_width(value)) * 1233 >> 12;
        return approx_log10 + static_cast<unsigned int>(value >= pow_table<10>::data.pow[approx_log10 - 1]);
    }
#endif
    if (base == 2) return std::bit_width(value);
    if (std::has_single_bit(base)) { 
        // another power of 2 bases (4, 8, 16, 32, 64) - can be computed in constant time using bit_width (log2)
        const auto bp = static_cast<unsigned int>(std::bit_width(base - 1));
        return (static_cast<unsigned int>(std::bit_width(value)) + bp - 1) / bp;
    }

    // fallback to slow method
    unsigned int len = 1;
    const unsigned int base2 = base * base;
    const unsigned int base4 = base2 * base2;
    for (; value >= base4; value /= base4) len += 4;
    for (; value >= base2; value /= base2) len += 2;
    for (; value >= base; value /= base) ++len;
    return len;
}

constexpr unsigned int integral_length(std::intmax_t value, unsigned int base) {
    return static_cast<unsigned int>(value < 0) + unsigned_integral_length(integral_abs(value), base);
}

template <unsigned int base, unsigned int digits, bool upper = false>
void unsigned_integral_lookup_chars(char* buf, unsigned int len, std::uintmax_t value) {
    using lookup = integral_lookup<base, digits, upper>;
    const typename lookup::table_data& l = lookup::data;
    for (; len >= digits; value /= lookup::pow) {
        len -= digits;
        std::memcpy(buf + len, l.chars[value % lookup::pow], digits);
    }
    if (len > 0) std::memcpy(buf, l.chars[value % lookup::pow] + digits - len, len);
}

inline void unsigned_integral_chars(char* buf, unsigned int len, std::uintmax_t value, unsigned int base, bool upper) {
    // all lookup tables requires ~1,5kb of memory
    // but performance is almost the same for small numbers (~<1000, base 10)
    // the biggest impact - optimize binary (base=2) values
#if (ANSIPP_FAST_INTEGRAL & 0x3f) != 0
    switch (base) {
#if (ANSIPP_FAST_INTEGRAL & 0x01) != 0
        // 2^4*4 = 64 bytes
        case 2: 
            unsigned_integral_lookup_chars<2, 4>(buf, len, value); 
            return;
#endif
#if (ANSIPP_FAST_INTEGRAL & 0x02) != 0
        // 8^2*2 = 128 bytes
        case 8: 
            unsigned_integral_lookup_chars<8, 2>(buf, len, value); 
            return;
#endif
#if (ANSIPP_FAST_INTEGRAL & 0x04) != 0
        // 10^2*2 = 200 bytes
        [[likely]] case 10: 
            unsigned_integral_lookup_chars<10, 2>(buf, len, value); 
            return;
#endif
#if (ANSIPP_FAST_INTEGRAL & (0x08 | 0x10)) != 0
        // 16^2*2 = 512 bytes per each table, 1024 for both
        // generally HEX values quite fast even without tables due to big base value
        case 16: 
            if (upper) {
#if (ANSIPP_FAST_INTEGRAL & 0x08) != 0
                unsigned_integral_lookup_chars<16, 2, true>(buf, len, value); 
                return;
#endif
            } else {
#if (ANSIPP_FAST_INTEGRAL & 0x10) != 0                
                unsigned_integral_lookup_chars<16, 2, false>(buf, len, value); 
                return;
#endif
            }
#endif
    }
#endif
    for (char* ptr = buf + len; ptr != buf; value /= base) {
        *--ptr = to_digit(value % base, upper); 
    }
}

inline void integral_chars(char* buf, unsigned int length, std::intmax_t value, unsigned int base, bool upper) {
    if (value < 0) { *buf++ = '-'; --length; }
    unsigned_integral_chars(buf, length, integral_abs(value), base, upper); 
}

}