#pragma once

#include <type_traits>
#include <concepts>

namespace ansipp {

inline char to_digit(unsigned int v, bool upper) { return static_cast<char>((v < 10 ? '0' : upper ? 'A' : 'a') + v); }

template <std::integral T>
std::make_unsigned_t<T> unsigned_integral_abs(T v) {
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
unsigned int unsigned_integral_length(T value, unsigned int base) {
    unsigned int len = 1;
    for (; value >= base; value /= base, ++len);
    return len;
}

template <std::unsigned_integral T>
void unsigned_integral_chars(char* buf, unsigned int length, T value, unsigned int base, bool upper) {
    for (char* ptr = buf + length; ptr != buf; value /= base) *--ptr = to_digit(value % base, upper);
    //for (char* ptr = buf; ptr != buf + length; ++ptr, value /= base) *ptr = to_digit(value % base, upper);
}

template <std::integral T>
unsigned int integral_length(T value, unsigned int base) {
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