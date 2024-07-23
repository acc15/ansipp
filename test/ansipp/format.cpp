#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark_all.hpp>

#if __has_include(<format>)
#   include <format>
#endif
#include <string>
#include <sstream>
#include <charconv>

#include <ansipp/cursor.hpp>
#include <ansipp/charbuf.hpp>

using namespace ansipp;

struct format_benchmark {

    static constexpr int x = 5;
    static constexpr int y = 10;
    static constexpr char expected_esc[] = "\33" "[10;5H";
    static constexpr std::size_t min_buf_size = std::size(expected_esc);

    char buf[min_buf_size];
    std::string shared_str = std::string(min_buf_size, 0);
    charbuf shared_cb = charbuf(min_buf_size);
    std::stringstream shared_ss;

#if __has_include(<format>)
    std::string std_format() { 
        return std::format("{}{};{}H", csi, y, x); 
    }

    std::string std_format_shared() { 
        char* end = std::format_to(buf, "{}{};{}H", csi, y, x); 
        return std::string(buf, end);
    }
#endif

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
        int len = snprintf(buf, sizeof(buf), "%s%u;%uH", csi.c_str(), y, x);
        return std::string(buf, len);
    }

    std::string charbuf_alloc() {
        charbuf cb;
        cb << csi << y << ';' << x << 'H';
        return cb.str();
    }

    std::string charbuf_shared() {
        shared_cb.reset() << csi << y << ';' << x << 'H';
        return shared_cb.str();
    }

    std::string std_to_chars() {
        char* ptr = buf;
        ptr += csi.copy(ptr, csi.size());
        ptr = std::to_chars(ptr, std::end(buf), y).ptr;
        *ptr++ = ';';
        ptr = std::to_chars(ptr, std::end(buf), x).ptr;
        *ptr++ = 'H';
        return std::string(buf, ptr);
    }

    std::string current_impl() {
        shared_cb.reset() << move_abs(x, y);
        return shared_cb.str();
    }

};

// Main benchmark for escape codes format decisions
// it compares different formatting methods
TEST_CASE("format: benchmark", "[format][!benchmark]") {
    
    format_benchmark fb;
    
#if __has_include(<format>)
    REQUIRE( fb.std_format() == fb.expected_esc );
    REQUIRE( fb.std_format_shared() == fb.expected_esc );
#endif
    REQUIRE( fb.std_string_append() == fb.expected_esc );
    REQUIRE( fb.std_string_append_shared() == fb.expected_esc );
    REQUIRE( fb.std_stringstream() == fb.expected_esc );
    REQUIRE( fb.std_stringstream_shared() == fb.expected_esc );
    REQUIRE( fb.snprintf_shared() == fb.expected_esc );
    REQUIRE( fb.std_to_chars() == fb.expected_esc );
    REQUIRE( fb.charbuf_alloc() == fb.expected_esc );
    REQUIRE( fb.charbuf_shared() == fb.expected_esc );
    REQUIRE( fb.current_impl() == fb.expected_esc ); 

#if __has_include(<format>)
    BENCHMARK("std_format") { return fb.std_format(); }; 
    BENCHMARK("std_format_shared") { return fb.std_format_shared(); }; 
#endif
    BENCHMARK("std_string_append") { return fb.std_string_append(); };
    BENCHMARK("std_string_append_shared") { return fb.std_string_append_shared(); };
    BENCHMARK("std_stringstream") { return fb.std_stringstream(); };
    BENCHMARK("std_stringstream_shared") { return fb.std_stringstream_shared(); };
    BENCHMARK("snprintf_shared") { return fb.snprintf_shared(); };
    BENCHMARK("std_to_chars") { return fb.std_to_chars(); };
    BENCHMARK("charbuf") { return fb.charbuf_alloc(); };
    BENCHMARK("charbuf_shared") { return fb.charbuf_shared(); };
    BENCHMARK("current_impl") { return fb.current_impl(); };

}
