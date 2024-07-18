#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark_all.hpp>
#include <ansipp/cursor.hpp>

#include <bit>
#include <new>
#include <cstdlib>

using namespace ansipp;

std::size_t uint_len(unsigned int value) {
    return value < 100000 
        ? value < 1000       ? value < 10      ? 1 : value < 100      ? 2 : 3 : value < 10000       ? 4 : 5
        : value < 100000000  ? value < 1000000 ? 6 : value < 10000000 ? 7 : 8 : value < 1000000000  ? 9 : 10;
}

char to_digit(unsigned int v, bool upper) { return v < 10 ? '0' + v : (upper ? 'A' : 'a') + v; }
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

    charbuf& clear() { off = 0; return *this; }
    char* data() { return ptr; }
    const char* data() const { return ptr; }
    std::size_t capacity() const { return sz; }
    std::size_t offset() const { return off; }
    std::string_view view() const { return std::string_view(ptr, off); }
    std::string str() const { return std::string(ptr, off); }
    charbuf& put(const void* data, std::size_t size) { std::memcpy(reserve(size), data, size); return *this; }
    charbuf& put(std::string_view str) { return put(str.data(), str.size()); }
    charbuf& put(char ch) { return put(&ch, 1); }
};

charbuf& operator<<(charbuf& buf, std::string_view sv) { return buf.put(sv); }
charbuf& operator<<(charbuf& buf, char c) { return buf.put(c); }
charbuf& operator<<(charbuf& buf, unsigned int v) {
    if (v < 10) { return buf.put('0' + v); }
    std::size_t len = uint_len(v);
    char* data = buf.reserve(len);
    uint_puts(data, len, v, 10, false);
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

struct format_benchmark {

    const unsigned int x = 5;
    const unsigned int y = 10;
    const std::string expected_esc = "\33" "[10;5H";

    char snprintf_buf[20];
    std::string shared_str = std::string(20, 0);
    charbuf shared_cb = charbuf(20);
    std::stringstream shared_ss;

    std::string std_format() { 
        return std::format("{}{};{}H", csi, y, x); 
    }

    std::string std_string_append() { 
        return std::string(csi)
            .append(std::to_string(y)).append(1, ';')
            .append(std::to_string(x)).append(1, 'H');  
    }

    std::string std_string_append_shared() { 
        return shared_str.assign(csi)
            .append(std::to_string(y)).append(1, ';')
            .append(std::to_string(x)).append(1, 'H'); 
    }

    std::string std_stringstream() { 
        std::stringstream ss;
        ss << csi << y << ';' << x << 'H';
        return ss.str();
    }

    std::string std_stringstream_shared() { 
        shared_ss.str("");
        shared_ss << csi << y << ';' << x << 'H';
        return shared_ss.str();
    }
    std::string snprintf_shared() {
        int len = snprintf(snprintf_buf, sizeof(snprintf_buf), "%s%u;%uH", csi.c_str(), y, x);
        return std::string(snprintf_buf, len);
    }

    std::string charbuf_alloc() {
        charbuf cb(16);
        cb << csi << y << ';' << x << 'H';
        return cb.str();
    }

    std::string charbuf_shared() {
        shared_cb.clear() << csi << y << ';' << x << 'H';
        return shared_cb.str();
    }

    std::string current_impl() {
        return move_abs(x, y);
    }
    
};


TEST_CASE("format: benchmark", "[format][!benchmark]") {
    
    format_benchmark fb;
    
    REQUIRE( fb.std_format() == fb.expected_esc );
    REQUIRE( fb.std_string_append() == fb.expected_esc );
    REQUIRE( fb.std_string_append_shared() == fb.expected_esc );
    REQUIRE( fb.std_stringstream() == fb.expected_esc );
    REQUIRE( fb.std_stringstream_shared() == fb.expected_esc );
    REQUIRE( fb.snprintf_shared() == fb.expected_esc );
    REQUIRE( fb.charbuf_alloc() == fb.expected_esc );
    REQUIRE( fb.charbuf_shared () == fb.expected_esc );
    REQUIRE( fb.current_impl() == fb.expected_esc ); 

    BENCHMARK("std_format") { return fb.std_format(); }; 
    BENCHMARK("std_string_append") { return fb.std_string_append(); };
    BENCHMARK("std_string_append_shared") { return fb.std_string_append_shared(); };
    BENCHMARK("std_stringstream") { return fb.std_stringstream(); };
    BENCHMARK("std_stringstream_shared") { return fb.std_stringstream_shared(); };
    BENCHMARK("snprintf_shared") { return fb.snprintf_shared(); };
    BENCHMARK("charbuf") { return fb.charbuf_alloc(); };
    BENCHMARK("charbuf_shared") { return fb.charbuf_shared(); };
    BENCHMARK("current_impl") { return fb.current_impl(); };

}
