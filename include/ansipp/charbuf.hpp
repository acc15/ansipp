#pragma once

#include <cstdlib> // std::malloc, std::free, etc
#include <cstring> // std::memcpy
#include <bit> // std::bit_ceil
#include <new> // bad_alloc
#include <string>
#include <string_view>

#include <ansipp/integral.hpp>

namespace ansipp {

template <typename T>
struct integral_format {
    T value;
    unsigned int base;
    bool upper;
    unsigned int width;

    integral_format(T value, unsigned int base = 10, bool upper = false, unsigned int width = 0):
        value(value), base(base), upper(upper), width(width) {}
};

class charbuf {
    
    static constexpr std::size_t min_alloc_sz = 32;

    char* b;
    char* e;
    char* p;

    void resize(std::size_t ns) {
        ns = ns < min_alloc_sz ? min_alloc_sz : std::bit_ceil(ns);
        char* nb = static_cast<char*>(std::realloc(b, ns));
        if (nb == nullptr) [[unlikely]] throw std::bad_alloc();
        p = nb + (p - b);
        b = nb;
        e = b + ns;
    }

public:
    charbuf(): b(nullptr), e(nullptr), p(nullptr) {}
    charbuf(std::size_t initial_size): charbuf() { resize(initial_size); }
    charbuf(charbuf&& mv): b(mv.b), e(mv.e), p(mv.p) { mv.b = mv.e = mv.p = nullptr; }
    ~charbuf() { std::free(b); }

    char* reserve(std::size_t size) {
        if (char* np = p + size; np > e) [[unlikely]] resize(np - b);
        char* buf = p;
        p += size;
        return buf;
    }

    charbuf& operator=(charbuf&& mv) {
        std::free(b);
        b = mv.b; e = mv.e; p = mv.p;
        mv.b = mv.e = mv.p = nullptr;
        return *this;
    }

    charbuf& reset() { p = b; return *this; }
    char* begin() { return b; }
    const char* begin() const { return b; }
    char* end() { return e; }
    const char* end() const { return e; }
    char* data() { return b; }
    const char* data() const { return b; }
    std::size_t size() const { return e - b; }
    std::size_t offset() const { return p - b; }
    std::string_view view() const { return std::string_view(b, p); }
    std::string_view flush() { std::string_view r(b, p); p = b; return r; }
    std::string str() const  { return std::string(b, p); }
    
    charbuf& put(const void* data, std::size_t size) { std::memcpy(reserve(size), data, size); return *this; }
    charbuf& put(char ch) { 
        if (p >= e) [[unlikely]] resize(e - b + 1);
        *p++ = ch;
        return *this;
    }
    
    charbuf& put(char ch, std::size_t count) { std::memset(reserve(count), ch, count); return *this; }

    template <typename T>
    charbuf& operator<<(const T& v) && { return *this << v; }

    charbuf& operator<<(char c) { return put(c); }
    charbuf& operator<<(bool v) { return put(v ? '1' : '0'); }
    charbuf& operator<<(const char* sv) { return put(sv, std::strlen(sv)); }
    charbuf& operator<<(std::string_view sv) { return put(sv.data(), sv.size()); }

    template <std::integral T>
    charbuf& operator<<(const integral_format<T>& v) {
        unsigned int width = v.width == 0 ? integral_length(v.value, v.base) : v.width;
        integral_chars(reserve(width), width, v.value, v.base, v.upper);
        return *this;
    }

    template <std::integral T>
    charbuf& operator<<(T v) { 
        unsigned int w = integral_length(v, 10);
        integral_chars(reserve(w), w, v, 10, false);
        return *this;
    }

};

}