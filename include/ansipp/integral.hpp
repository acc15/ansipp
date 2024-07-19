#pragma once

#include <type_traits>
#include <concepts>

namespace ansipp {

inline char to_digit(unsigned int v, bool upper) { return static_cast<char>((v < 10 ? '0' : upper ? 'A' : 'a') + v); }

template <std::unsigned_integral T>
void unsigned_integral_chars(char* buf, unsigned int digits, T v, unsigned int base, bool upper) {
    for (unsigned int i = 0; i < digits; ++i, v /= base) buf[digits - i - 1] = to_digit(v % base, upper);
}

template <std::unsigned_integral T>
unsigned int unsigned_digit_count(T value, unsigned int base) {
    unsigned int len = 1;
    for (; value >= base; value /= base, ++len);
    return len;
}

template <std::integral T>
std::make_unsigned_t<T> unsigned_abs(T v) {
    if constexpr (std::is_signed_v<T>) {
        const auto uv = std::make_unsigned_t<T>(v);
        return v < 0 ? -uv : uv;
    } else {
        return v;
    }
}

}