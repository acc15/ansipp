#pragma once

#include <type_traits>
#include <concepts>

namespace ansipp {

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

constexpr char to_digit(unsigned int v, bool upper) { return static_cast<char>((v < 10 ? '0' : upper ? 'A' : 'a') + v); }

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
constexpr unsigned int unsigned_integral_length(T value, unsigned int base) {
    unsigned int base2 = base * base;
    unsigned int base4 = base2 * base2;
    unsigned int len = 1;
    for (; value >= base4; value /= base4, len += 4);
    for (; value >= base; value /= base, ++len);
    return len;
}

template <std::unsigned_integral T>
void unsigned_integral_chars(char* buf, unsigned int length, T value, unsigned int base, bool upper) {
    for (char* ptr = buf + length; value >= base; value /= base) *--ptr = to_digit(value % base, upper);
    *buf = to_digit(value, upper);
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