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
    
    static constexpr std::size_t min_alloc_sz = 16;

    char* begin;
    char* end;
    char* ptr;

    void resize(std::size_t new_sz) {
        new_sz = new_sz < min_alloc_sz ? min_alloc_sz : std::bit_ceil(new_sz);
        char* new_begin = static_cast<char*>(std::realloc(begin, new_sz));
        if (new_begin == nullptr) [[unlikely]] throw std::bad_alloc();
        ptr = new_begin + (ptr - begin);
        begin = new_begin;
        end = begin + new_sz;
    }

public:
    charbuf(): begin(nullptr), end(nullptr), ptr(nullptr) {}

    charbuf(std::size_t initial_size) {
        std::size_t sz = initial_size < min_alloc_sz ? min_alloc_sz : std::bit_ceil(initial_size);
        begin = static_cast<char*>(std::malloc(sz));
        if (begin == nullptr) throw std::bad_alloc();
        end = begin + sz;
        ptr = begin;
    }

    charbuf(charbuf&& mv): begin(mv.begin), end(mv.end), ptr(mv.ptr) { mv.begin = mv.end = mv.ptr = nullptr; }
    ~charbuf() { std::free(begin); }

    char* reserve(std::size_t size) {
        if (char* new_ptr = ptr + size; new_ptr > end) [[unlikely]] resize(new_ptr - begin);
        char* buf = ptr;
        ptr += size;
        return buf;
    }

    charbuf& operator=(charbuf&& mv) {
        std::free(ptr);
        begin = mv.begin; end = mv.end; ptr = mv.ptr;
        mv.begin = mv.end = mv.ptr = nullptr;
        return *this;
    }

    charbuf& clear() { ptr = begin; return *this; }
    char* data() { return ptr; }
    const char* data() const { return ptr; }
    std::size_t capacity() const { return end - begin; }
    std::size_t offset() const { return ptr - begin; }
    std::string_view view() const { return std::string_view(begin, ptr); }
    std::string str() const  { return std::string(begin, ptr); }
    
    charbuf& put(const void* data, std::size_t size) {
        std::memcpy(reserve(size), data, size); 
        return *this;
    }

    charbuf& put(char ch) {
        if (ptr + 1 > end) [[unlikely]] resize(ptr - begin + 1);
        *ptr++ = ch;
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