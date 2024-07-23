#pragma once

#include <cstdlib> // std::malloc, std::free, etc
#include <cstring> // std::memcpy
#include <bit> // std::bit_ceil
#include <new> // bad_alloc
#include <string>
#include <string_view>

#include <ansipp/integral.hpp>

namespace ansipp {

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
    std::string str() const  { return std::string(b, p); }
    
    charbuf& put(const void* data, std::size_t size) { std::memcpy(reserve(size), data, size); return *this; }
    charbuf& put(char ch) {
        if (p + 1 > e) [[unlikely]] resize(p - b + 1);
        *p++ = ch;
        return *this;
    }

    charbuf& fill(std::size_t count, char ch) { std::memset(reserve(count), ch, count); return *this; }

    template <typename T>
    charbuf& operator<<(const T& v) && { return *this << v; }

    charbuf& operator<<(char c) { return put(c); }
    charbuf& operator<<(bool v) { return put(v ? '1' : '0'); }
    charbuf& operator<<(const char* sv) { return put(sv, strlen(sv)); }
    charbuf& operator<<(std::string_view sv) { return put(sv.data(), sv.size()); }

    template <std::integral T>
    charbuf& operator<<(T v) { 
        constexpr unsigned int base = 10;
        unsigned int len = integral_length(v, base);
        integral_chars(reserve(len), len, v, base, false);
        return *this;
    }

};

}