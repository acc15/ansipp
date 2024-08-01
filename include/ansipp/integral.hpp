#pragma once

#include <cstdint>
#include <type_traits>
#include <concepts>
#include <limits>
#include <utility>
#include <algorithm>
#include <bit>

// by default only decimals are optimized
#ifndef ANSIPP_FAST_INTEGRAL
#define ANSIPP_FAST_INTEGRAL 0x04
#endif

namespace ansipp {

constexpr char digit(unsigned int v, bool upper = false) { 
    return static_cast<char>((v < 10 ? '0' : upper ? 'A' - 10 : 'a' - 10) + v); 
}

template <typename T>
constexpr T ipow(T base, unsigned int pow) {
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
constexpr std::pair<unsigned int, T> imaxpow(T base) {
    unsigned int pow = 0;
    T v = 1;
    for (T nv = v*base; nv / base == v; ++pow, v = nv, nv *= base);
    return std::make_pair(pow, v);
}

template <unsigned int base, typename T = std::uintmax_t>
struct ipow_lookup {
    constexpr static unsigned int size = imaxpow<T>(base).first;
    struct table_data {
        T pow[size];
        constexpr table_data() {
            T v = base;
            for (unsigned int i = 0; i < size; pow[i] = v, v *= base, ++i);
        }
    };
    constexpr static table_data data = {};
};

template <unsigned int base, unsigned int digits, bool upper = false>
struct ichars_lookup {
    constexpr static unsigned int pow = ipow(base, digits);
    struct table_data {
        char chars[pow][digits];
        constexpr table_data() {
            for (unsigned int v = 0; v < pow; ++v) {
                for (unsigned int c = digits, n = v; c-- > 0; n /= base) {
                    chars[v][c] = digit(n % base, upper);
                }
            }
        }
    };
    constexpr static table_data data = {};
};

constexpr std::uintmax_t iabs(std::intmax_t v) {
    const auto uv = static_cast<std::uintmax_t>(v);
    if (v >= 0) return uv;
#ifdef _MSC_VER
#pragma warning(suppress: 4146)
    return -uv;
#else
    return -uv;
#endif
}

constexpr unsigned int ulen10(std::uintmax_t value) {
    // idea from http://www.graphics.stanford.edu/~seander/bithacks.html
    // requires ipow_lookup<10> table = sizeof(std::uintmax_t)*(std::numeric_limits<std::uintmax_t>::digits10)
    // if uintmax_t is 64 bit then 8*19 = 152 bytes
    if (value < 10) return 1;
    constexpr int bits = std::numeric_limits<std::uintmax_t>::digits - 1;
    unsigned int approx_log10 = (bits - std::countl_zero(value)) * 1233U >> 12U;
    return 1U + approx_log10 + static_cast<unsigned int>(value >= ipow_lookup<10>::data.pow[approx_log10]);
}

constexpr unsigned int ulen(std::uintmax_t value, unsigned int base) {
#if (ANSIPP_FAST_INTEGRAL & 0x04) != 0 
    if (base == 10) [[ likely ]] return ulen10(value);
#endif
    if (value < base) return 1;
    if (base == 2) return std::bit_width(value);
    if (std::has_single_bit(base)) { 
        // power of 2 bases (4, 8, 16, 32, 64) - can be computed in constant time using bit_width (log2)
        unsigned int bp = std::bit_width(base - 1U);
        return (std::bit_width(value) + bp - 1U) / bp;
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

constexpr unsigned int ilen(std::intmax_t value, unsigned int base) {
    return static_cast<unsigned int>(value < 0) + ulen(iabs(value), base);
}

template <unsigned int base, unsigned int digits, bool upper = false>
constexpr void ulookupchars(char* buf, unsigned int len, std::uintmax_t value) {
    using lookup = ichars_lookup<base, digits, upper>;
    const typename lookup::table_data& l = lookup::data;
    for (; len >= digits; value /= lookup::pow) {
        len -= digits;
        std::copy_n(l.chars[value % lookup::pow], digits, buf + len);
    }
    if (len > 0) std::copy_n(l.chars[value % lookup::pow] + digits - len, len, buf);
}

constexpr void uchars(char* buf, unsigned int len, std::uintmax_t value, unsigned int base, bool upper) {
    // all lookup tables requires ~1,5kb of memory
    // but performance is almost the same for small numbers (~<1000, base 10)
    // the biggest impact - optimize binary (base=2) values
#if (ANSIPP_FAST_INTEGRAL & 0x3f) != 0
    switch (base) {
#if (ANSIPP_FAST_INTEGRAL & 0x01) != 0
        // 2^4*4 = 64 bytes
        case 2: 
            ulookupchars<2, 4>(buf, len, value); 
            return;
#endif
#if (ANSIPP_FAST_INTEGRAL & 0x02) != 0
        // 8^2*2 = 128 bytes
        case 8: 
            ulookupchars<8, 2>(buf, len, value); 
            return;
#endif
#if (ANSIPP_FAST_INTEGRAL & 0x04) != 0
        // 10^2*2 = 200 bytes
        [[likely]] case 10: 
            ulookupchars<10, 2>(buf, len, value); 
            return;
#endif
#if (ANSIPP_FAST_INTEGRAL & (0x08 | 0x10)) != 0
        // 16^2*2 = 512 bytes per each table, 1024 for both
        // generally HEX values quite fast even without tables due to big base value
        case 16: 
            if (upper) {
#if (ANSIPP_FAST_INTEGRAL & 0x08) != 0
                ulookupchars<16, 2, true>(buf, len, value); 
                return;
#endif
            } else {
#if (ANSIPP_FAST_INTEGRAL & 0x10) != 0                
                ulookupchars<16, 2, false>(buf, len, value); 
                return;
#endif
            }
#endif
    }
#endif
    for (char* ptr = buf + len; ptr != buf; value /= base) {
        *--ptr = digit(value % base, upper); 
    }
}

constexpr void ichars(char* buf, unsigned int length, std::intmax_t value, unsigned int base, bool upper) {
    if (value < 0) { *buf++ = '-'; --length; }
    uchars(buf, length, iabs(value), base, upper); 
}

}