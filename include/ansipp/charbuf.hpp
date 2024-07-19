#pragma once

#include <cstdlib>
#include <bit>
#include <new>
#include <string>
#include <string_view>

#include <ansipp/integral.hpp>

namespace ansipp {

class charbuf {
    std::size_t sz;
    std::size_t off;
    char* ptr;

    void resize(std::size_t new_sz) {
        new_sz = std::bit_ceil(new_sz);
        void* new_ptr = std::realloc(ptr, new_sz);
        if (new_ptr == nullptr) [[unlikely]] throw std::bad_alloc();
        ptr = static_cast<char*>(new_ptr);
        sz = new_sz;
    }

public:
    charbuf(): sz(0), off(0), ptr(nullptr) {}

    charbuf(std::size_t initial_size): 
        sz(std::bit_ceil(initial_size)), off(0), ptr(static_cast<char*>(std::malloc(sz))) 
    { if (ptr == nullptr) throw std::bad_alloc(); }

    charbuf(charbuf&& mv): sz(mv.sz), off(mv.off), ptr(mv.ptr) { mv.sz = 0; mv.off = 0; mv.ptr = nullptr; }
    ~charbuf() { std::free(ptr); }

    char* reserve(std::size_t size) {
        std::size_t new_off = off + size;
        if (new_off > sz) [[unlikely]] resize(new_off);
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
    std::string str() const  { return std::string(ptr, off); }
    
    charbuf& put(const void* data, std::size_t size) { 
        std::memcpy(reserve(size), data, size); 
        return *this;    
    }

    charbuf& put(char ch) { 
        if (off + 1 > sz) [[unlikely]] resize(off + 1);
        *(ptr + off) = ch;
        ++off;
        return *this;
    }

    charbuf& fill(std::size_t count, char ch) { 
        std::memset(reserve(count), ch, count);
        return *this;
    }

};

inline charbuf& operator<<(charbuf& buf, char c) { buf.put(c); return buf; }
inline charbuf& operator<<(charbuf& buf, bool v) { buf.put(v ? '1' : '0'); return buf; }
inline charbuf& operator<<(charbuf& buf, const char* sv) { buf.put(sv, strlen(sv)); return buf; }
inline charbuf& operator<<(charbuf& buf, std::string_view sv) { buf.put(sv.data(), sv.size()); return buf; }

template <std::integral T>
inline charbuf& operator<<(charbuf& buf, const T& v) { 
    if constexpr (std::is_signed_v<T>) if (v < 0) buf.put('-');
    const auto uv = unsigned_abs(v);
    unsigned int len = unsigned_digit_count(uv, 10);
    unsigned_integral_chars(buf.reserve(len), len, uv, 10, false);
    return buf;
}

}