#pragma once

#include <type_traits>
#include <concepts>
#include <cstring>

namespace ansipp {

constexpr char to_digit(unsigned int v, bool upper = false) { 
    return static_cast<char>((v < 10 ? '0' : upper ? 'A' : 'a') + v); 
}

constexpr unsigned int cpow(unsigned int base, unsigned int pow) {
    unsigned int result = 1;
    while (pow > 0) {
        if ((pow & 1) == 0) { 
            pow >>= 1; 
            base *= base; 
        } else {
            --pow;
            result *= base;
        }
    }
    return result;
}

template <unsigned int base, unsigned int digits, bool upper = false>
struct integral_lookup {
    constexpr static unsigned int pow = cpow(base, digits);
    char chars[pow][digits];
    constexpr integral_lookup() {
        for (unsigned int v = 0; v < pow; ++v) {
            unsigned int n = v;
            for (unsigned int c = digits; c-- > 0; n /= base) {
                chars[v][c] = to_digit(n % base, upper);
            }
        }
    }

    static const integral_lookup instance;
};

template <unsigned int base, unsigned int digits, bool upper>
constexpr integral_lookup<base, digits, upper> integral_lookup<base, digits, upper>::instance;

template <std::unsigned_integral T, unsigned int base, unsigned int digits, bool upper = false>
void unsigned_integral_lookup_chars(char* buf, unsigned int len, T value) {
    using lookup = integral_lookup<base, digits, upper>;
    const lookup& l = lookup::instance;
    // char* ptr = buf + len;
    // for (; value >= lookup::pow; value /= lookup::pow) {
    //     ptr -= digits;
    //     std::memcpy(ptr, l.chars[value % lookup::pow], digits);
    // }
    // for (; value >= base; value /= base) *--ptr = to_digit(value % base, upper);
    // *--ptr = to_digit(value, upper);

    // char* ptr = buf + len;
    // for (; ptr - digits >= buf; value /= lookup::pow) {
    //     ptr -= digits;
    //     std::memcpy(ptr, l.chars[value % lookup::pow], digits);
    // }
    // if (len > 0) std::memcpy(buf, l.chars[value % lookup::pow] + digits - len, len);

    for (; len >= digits; value /= lookup::pow) {
        len -= digits;
        std::memcpy(buf + len, l.chars[value % lookup::pow], digits);
    }
    if (len > 0) std::memcpy(buf, l.chars[value % lookup::pow] + digits - len, len);
}

template <std::integral T>
constexpr std::make_unsigned_t<T> unsigned_integral_abs(T v) {
    if constexpr (std::is_signed_v<T>) {
        const auto uv = std::make_unsigned_t<T>(v);
        if (v < 0) {
#ifdef _MSC_VER
#pragma warning(suppress: 4146)
            return -uv;
#else
            return -uv;
#endif
        }
        return uv;
    } else {
        return v;
    }
}

template <std::unsigned_integral T>
constexpr unsigned int unsigned_integral_length(T value, unsigned int radix) {
    unsigned int len = 1;
    if constexpr (std::numeric_limits<T>::digits > 8) {
        unsigned int radix2 = radix * radix;
        if constexpr (std::numeric_limits<T>::digits > 16) {
            unsigned int radix4 = radix2 * radix2;
            for (; value >= radix4; value /= radix4) len += 4;
        }
        for (; value >= radix2; value /= radix2) len += 2;
    }
    for (; value >= radix; value /= radix) ++len;
    return len;
}

template <std::unsigned_integral T>
void unsigned_integral_chars(char* buf, unsigned int len, T value, unsigned int base, bool upper) {
    switch (base) {
        case 10: unsigned_integral_lookup_chars<T, 10, 2>(buf, len, value); return;
        case  2: unsigned_integral_lookup_chars<T, 2, 4>(buf, len, value); return;
        case  8: unsigned_integral_lookup_chars<T, 8, 2>(buf, len, value); return;
        case 16: 
            if (upper) {
                unsigned_integral_lookup_chars<T, 16, 2, true>(buf, len, value); 
            } else {
                unsigned_integral_lookup_chars<T, 16, 2, false>(buf, len, value); 
            }
            return;
        default: 
            for (char* ptr = buf + len; ptr != buf; value /= base) *--ptr = to_digit(value % base, upper); 
            return;
    }
}

template <std::integral T>
constexpr unsigned int integral_length(T value, unsigned int base) {
    unsigned int unsigned_length = unsigned_integral_length(unsigned_integral_abs(value), base);
    if constexpr (std::is_signed_v<T>) if (value < 0) return unsigned_length + 1;
    return unsigned_length;
}

template <std::integral T>
void integral_chars(char* buf, unsigned int length, T value, unsigned int base, bool upper) {
    if constexpr (std::is_signed_v<T>) if (value < 0) { *buf++ = '-'; --length; }
    unsigned_integral_chars(buf, length, unsigned_integral_abs(value), base, upper); 
}

}