#pragma once

#include <cstdlib>
#include <cstring>
#include <bit>
#include <new>
#include <string>
#include <string_view>

namespace ansipp {


class charbuf {
    std::size_t sz;
    std::size_t off;
    char* ptr;
public:
    charbuf(): sz(0), off(0), ptr(nullptr) {}
    charbuf(std::size_t initial_size): 
        sz(std::bit_ceil(initial_size)), 
        off(0), 
        ptr(static_cast<char*>(std::malloc(sz))) 
    { if (ptr == nullptr) throw std::bad_alloc(); }

    charbuf(charbuf&& mv): sz(mv.sz), off(mv.off), ptr(mv.ptr) {
        mv.sz = 0; mv.off = 0; mv.ptr = nullptr;
    }

    ~charbuf() { std::free(ptr); }

    char* reserve(std::size_t size) {
        std::size_t new_off = off + size;
        if (new_off > sz) {
            std::size_t new_sz = std::bit_ceil(new_off);
            void* new_ptr = std::realloc(ptr, new_sz);
            if (new_ptr == nullptr) throw std::bad_alloc();
            ptr = static_cast<char*>(new_ptr);
            sz = new_sz;
        }
        char* buf = ptr + off;
        off = new_off;
        return buf;
    }

    charbuf& operator=(charbuf&& mv) {
        std::free(ptr);
        sz = mv.sz; off = mv.off; ptr = mv.ptr;
        mv.sz = 0; mv.off = 0; mv.ptr = nullptr;
        return *this;
    }

    charbuf& clear() { off = 0; return *this; }
    char* data() { return ptr; }
    const char* data() const { return ptr; }
    std::size_t capacity() const { return sz; }
    std::size_t offset() const { return off; }
    std::string_view view() const { return std::string_view(ptr, off); }
    std::string str() const { return std::string(ptr, off); }
    charbuf& put(const void* data, std::size_t size) { std::memcpy(reserve(size), data, size); return *this; }

};

inline charbuf& operator<<(charbuf& buf, const char& c) { return buf.put(&c, 1); }
inline charbuf& operator<<(charbuf& buf, const bool& v) { return buf << (v ? '1' : '0'); }
inline charbuf& operator<<(charbuf& buf, const char* sv) { return buf.put(sv, strlen(sv)); }
inline charbuf& operator<<(charbuf& buf, std::string_view sv) { return buf.put(sv.data(), sv.size()); }

inline char to_digit(unsigned int v, bool upper) { return v < 10 ? '0' + v : (upper ? 'A' : 'a') + v; }

template <typename T>
void unsigned_integral_puts(char* buf, unsigned int len, T v, unsigned int base, bool upper) {
    for (unsigned int i = 0; i < len; ++i, v /= base) buf[len - i - 1] = to_digit(v % base, upper);
}

template <typename T>
unsigned int unsigned_digits_len(T value, unsigned int base) {
    unsigned int len = 1;
    for (; value >= base; value /= base, ++len);
    return len;
}

template <typename T>
std::make_unsigned_t<T> uabs(T v) {
    if constexpr (std::is_signed_v<T>) {
        using unsigned_t = std::make_unsigned_t<T>;
        return v < 0 ? -static_cast<unsigned_t>(v) : static_cast<unsigned_t>(v);
    } else {
        return v;
    }
}

template <typename T>
void integral_puts(charbuf& buf, T v, unsigned int base, bool upper) {
    const auto uv = uabs(v);
    unsigned int len = unsigned_digits_len(uv, base);
    char* data;
    if constexpr (std::is_signed_v<T>) {
        if (v < 0) buf << '-';
    } 
    data = buf.reserve(len);
    unsigned_integral_puts(data, len, uv, base, upper);
}

template <std::integral T>
inline charbuf& operator<<(charbuf& buf, const T& v) { 
    integral_puts(buf, v, 10, true);
    return buf;
}

}