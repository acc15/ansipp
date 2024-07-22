#pragma once

#include <cstdlib>
#include <cstring>
#include <bit>
#include <new>
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

    charbuf(std::size_t initial_size) {
        std::size_t s = initial_size < min_alloc_sz ? min_alloc_sz : std::bit_ceil(initial_size);
        b = static_cast<char*>(std::malloc(s));
        if (b == nullptr) throw std::bad_alloc();
        e = b + s;
        p = b;
    }

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
    
    charbuf& put(const void* data, std::size_t size) {
        std::memcpy(reserve(size), data, size); 
        return *this;
    }

    charbuf& put(char ch) {
        if (p + 1 > e) [[unlikely]] resize(p - b + 1);
        *p++ = ch;
        return *this;
    }

    charbuf& fill(std::size_t count, char ch) { 
        std::memset(reserve(count), ch, count);
        return *this;
    }

};

inline charbuf& operator<<(charbuf& buf, char c) { return buf.put(c); }
inline charbuf& operator<<(charbuf& buf, bool v) { return buf.put(v ? '1' : '0'); }
inline charbuf& operator<<(charbuf& buf, const char* sv) { return buf.put(sv, strlen(sv)); }
inline charbuf& operator<<(charbuf& buf, std::string_view sv) { return buf.put(sv.data(), sv.size()); }

template <std::integral T>
inline charbuf& operator<<(charbuf& buf, const T& v) { 
    unsigned int len = integral_length(v, 10);
    integral_chars(buf.reserve(len), len, v, 10, false);
    return buf;
}

template <typename T>
inline charbuf& operator<<(charbuf&& buf, const T& v) { 
    return buf << v;
}

}