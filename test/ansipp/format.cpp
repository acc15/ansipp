#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark_all.hpp>
#include <ansipp/cursor.hpp>

#include <bit>
#include <new>
#include <cstdlib>

using namespace ansipp;

inline std::size_t uint_len(unsigned int value) {
    return value < 100000 
        ? value < 1000       ? value < 10      ? 1 : value < 100      ? 2 : 3 : value < 10000       ? 4 : 5
        : value < 100000000  ? value < 1000000 ? 6 : value < 10000000 ? 7 : 8 : value < 1000000000  ? 9 : 10;
}

inline char to_digit(unsigned int v, bool upper) { return v < 10 ? '0' + v : (upper ? 'A' : 'a') + v; }

void uint_puts(char* buf, size_t len, unsigned int v, unsigned int base, bool upper) {
    for (std::size_t i = len - 1; v > 0; --i, v /= base) {
        buf[i] = to_digit(v % base, upper);
    }
}

class charbuf {
    std::size_t sz;
    std::size_t off;
    char* ptr;
public:
    charbuf(std::size_t initial_size = 4096): 
        sz(std::bit_ceil(initial_size)), 
        off(0), 
        ptr(static_cast<char*>(std::malloc(sz))) 
    { if (ptr == nullptr) throw std::bad_alloc(); }

    charbuf(charbuf&& mv): sz(mv.sz), off(mv.off), ptr(mv.ptr) {
        mv.sz = 0;
        mv.off = 0;
        mv.ptr = nullptr;
    }

    ~charbuf() { if (ptr != nullptr) std::free(ptr); }

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

    inline charbuf& clear() { off = 0; return *this; }
    inline char* data() { return ptr; }
    inline const char* data() const { return ptr; }
    inline std::size_t capacity() const { return sz; }
    inline std::size_t offset() const { return off; }
    inline std::string_view view() const { return std::string_view(ptr, off); }

    inline charbuf& put(const void* data, std::size_t size) { std::memcpy(reserve(size), data, size); return *this; }
    inline charbuf& put(std::string_view str) { return put(str.data(), str.size()); }
    inline charbuf& put(char ch) { return put(&ch, 1); }
};

charbuf& operator<<(charbuf& buf, std::string_view sv) { return buf.put(sv); }
charbuf& operator<<(charbuf& buf, char c) { return buf.put(c); }
charbuf& operator<<(charbuf& buf, unsigned int v) {
    if (v < 10) { return buf.put('0' + v); }
    std::size_t len = uint_len(v);
    char* data = buf.reserve(len);
    for (std::size_t i = len - 1; v > 0; --i, v /= 10) data[i] = '0' + (v % 10);
    return buf;
}

TEST_CASE("format: uint_len", "[format]") {
    REQUIRE( uint_len(1) == 1 );
    REQUIRE( uint_len(11) == 2 );
    REQUIRE( uint_len(111) == 3 );
    REQUIRE( uint_len(1111) == 4 );
    REQUIRE( uint_len(11111) == 5 );
    REQUIRE( uint_len(111111) == 6 );
    REQUIRE( uint_len(1111111) == 7 );
    REQUIRE( uint_len(11111111) == 8 );
    REQUIRE( uint_len(111111111) == 9 );
    REQUIRE( uint_len(1111111111) == 10 );
}

TEST_CASE("format: uint_puts", "[format]") {
    char buf[10];
    uint_puts(buf, uint_len(1553), 1553, 10, false);
    REQUIRE( buf[0] == '1' );
    REQUIRE( buf[1] == '5' );
    REQUIRE( buf[2] == '5' );
    REQUIRE( buf[3] == '3' );
}

TEST_CASE("format: cursor position", "[format][!benchmark]") {
    
    unsigned int x = 5, y = 10;

    std::string shared_str;
    char printf_buf[20];
    shared_str.reserve(20);

    charbuf shared_cb(16);

    BENCHMARK("std::format") {
        return std::format("{}{};{}H", csi, y, x);
    };
    BENCHMARK("std::string.append") {
        return std::string(csi)
            .append(std::to_string(y)).append(1, ';')
            .append(std::to_string(x)).append(1, 'H');
    };
    BENCHMARK("std::string.append shared") {
        return shared_str.assign(csi)
            .append(std::to_string(y)).append(1, ';')
            .append(std::to_string(x)).append(1, 'H');
    };
    BENCHMARK("std::stringstream") {
        std::stringstream ss;
        ss << csi << y << ';' << x << 'H';
        return ss.str();
    };
    BENCHMARK("snprintf") {
        return snprintf(printf_buf, sizeof(printf_buf), "%s%u;%uH", csi.c_str(), y, x);
    };
    BENCHMARK("charbuf") {
        charbuf cb(16);
        cb << csi << y << ';' << x << 'H';
        return cb.view();
    };
    BENCHMARK("charbuf shared") {
        shared_cb.clear() << csi << y << ';' << x << 'H';
        return shared_cb.view();
    };
    BENCHMARK("current impl") {
        return move_abs(x, y);
    };
}
