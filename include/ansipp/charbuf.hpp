#pragma once

#include <cstdlib> // std::malloc, std::free, etc
#include <cstring> // std::memcpy
#include <bit> // std::bit_ceil
#include <new> // bad_alloc
#include <string>
#include <string_view>
#include <charconv>
#include <concepts>

#include <ansipp/integral.hpp>
#include <ansipp/io.hpp>

namespace ansipp {

template <std::integral T>
struct integral_format {
    T value;
    unsigned int base;
    bool upper;
    unsigned int width;

    integral_format(T value, unsigned int base = 10, bool upper = false, unsigned int width = 0):
        value(value), base(base), upper(upper), width(width) {}
};

template <std::floating_point T>
struct floating_format {
    T value;
    std::chars_format format;
    int precision;

    floating_format(T value, std::chars_format format = std::chars_format::general, int precision = -1):
        value(value), format(format), precision(precision) {}
};

/**
 * Simple and fast mix of `std::string` and `std::stringstream`.
 * 
 * Why not `std::vector<char>`?
 * - Not uses `realloc` which sometimes can be faster than (`operator new[]`, `memory copy`, `operator delete[]`).
 * 
 * Why not `std::string`? 
 * - Same as `std::vector<char>`
 * - They are required now to zero memory when shrinking (`resize(0)` will result in redundant `memset` or similar loop) 
 *  for correct `.c_str()` implementation ()
 * - Some implementations may place '\0' on `append` instead of memset above - which is also redundant overhead
 */
class charbuf {
    
    static constexpr std::size_t min_alloc_sz = 32;

    char* b;
    char* e;
    char* p;

    void resize_pow2(std::size_t sz) {
        char* nb = static_cast<char*>(std::realloc(b, sz));
        if (nb == nullptr) [[unlikely]] throw std::bad_alloc();
        p = nb + (p - b);
        b = nb;
        e = b + sz;
    }

    void resize(std::size_t sz) {
        resize_pow2((std::max)(min_alloc_sz, std::bit_ceil(sz)));
    }

public:
    charbuf(): b(nullptr), e(nullptr), p(nullptr) {}
    charbuf(std::size_t initial_size): charbuf() { resize(initial_size); }
    charbuf(charbuf&& mv): b(mv.b), e(mv.e), p(mv.p) { mv.b = mv.e = mv.p = nullptr; }
    ~charbuf() { std::free(b); }

    void require(std::size_t size) {
        char* np = p + size;
        if (np > e) [[unlikely]] resize(np - b);
    }

    char* reserve(std::size_t size) {
        require(size);
        char* r = p;
        p += size;
        return r;
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
    std::size_t capacity() const { return e - b; }
    std::size_t size() const { return p - b; }
    std::string_view view() const { return std::string_view(b, p); }
    std::string_view flush() { char* pe = p; p = b; return std::string_view(b, pe); }
    std::string str() const { return std::string(b, p); }
    
    // compatibility with std::back_inserter
    void push_back(char ch) { 
        if (p >= e) [[unlikely]] resize(e - b + 1);
        *p++ = ch;
    }

    charbuf& put(const void* data, std::size_t size) { std::memcpy(reserve(size), data, size); return *this; }
    charbuf& put(char ch) { push_back(ch); return *this; }
    charbuf& put(char ch, std::size_t count) { std::memset(reserve(count), ch, count); return *this; }

    template <typename T>
    charbuf& operator<<(const T& v) && { return *this << v; }

    charbuf& operator<<(char c) { return put(c); }
    charbuf& operator<<(bool v) { return put(v ? '1' : '0'); }
    charbuf& operator<<(const char* sv) { return put(sv, std::strlen(sv)); }
    charbuf& operator<<(std::string_view sv) { return put(sv.data(), sv.size()); }

    template <std::unsigned_integral T>
    charbuf& operator<<(const integral_format<T>& v) {
        unsigned int width = v.width == 0 ? ulen(v.value, v.base) : v.width;
        uchars(reserve(width), width, v.value, v.base, v.upper);
        return *this;
    }

    template <std::unsigned_integral T>
    charbuf& operator<<(T v) { 
        unsigned int w = ulen(v, 10);
        uchars(reserve(w), w, v, 10, false);
        return *this;
    }

    template <std::signed_integral T>
    charbuf& operator<<(const integral_format<T>& v) {
        unsigned int width = v.width == 0 ? ilen(v.value, v.base) : v.width;
        ichars(reserve(width), width, v.value, v.base, v.upper);
        return *this;
    }

    template <std::signed_integral T>
    charbuf& operator<<(T v) { 
        unsigned int w = ilen(v, 10);
        ichars(reserve(w), w, v, 10, false);
        return *this;
    }
    
    template <std::floating_point T>
    charbuf& operator<<(const floating_format<T>& v) {
        require(1);

        std::size_t grow_by = 32 + (v.precision > 0 ? v.precision : 0);
        for (;;) {
            const std::to_chars_result r = v.precision < 0 
                ? std::to_chars(p, e, v.value, v.format)
                : std::to_chars(p, e, v.value, v.format, v.precision);
            if (r.ec != std::errc::value_too_large) {
                p = r.ptr;
                break;
            }
            require(grow_by);
            grow_by *= 2;
        }
        return *this;
    }

    template <std::floating_point T>
    charbuf& operator<<(T v) {
        return *this << floating_format(v);
    }

    charbuf& operator<<(void(*fn)(charbuf&)) { fn(*this); return *this; }

    static void reset(charbuf& b) { b.reset(); }
    static void to_stdout(charbuf& b) { stdout_write(b.flush()); }
    static void to_stderr(charbuf& b) { stderr_write(b.flush()); }

};

}