#include <version>
#include <limits>
#include <charconv>
#include <algorithm>
#include <cstdlib>

#if defined(__cpp_lib_format_ranges) || defined(__cpp_lib_format)
#   include <format>
#endif

#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark_all.hpp>
#include <catch2/generators/catch_generators_all.hpp>

#include <ansipp/integral.hpp>
#include "base_gen.hpp"

using namespace ansipp;

template <typename T>
void unsigned_integral_abs_with_limits() {
    DYNAMIC_SECTION(typeid(T).name()) {
        using limits = std::numeric_limits<T>;
        const T max = limits::max();
        REQUIRE( unsigned_integral_abs<T>(max) == max );
        REQUIRE( unsigned_integral_abs<T>(-1) == 1 );
    }
}

TEST_CASE("integral: unsigned comparisons", "[integral]") {
    unsigned char v = 255;

    unsigned int ui = 256;
    REQUIRE( v < ui );
    REQUIRE( ui >= v );
}

TEST_CASE("integral: to_digit", "[integral]") {
    std::string_view chars = "0123456789abcdefghijklmnopqrstuvwxyz";
    for (unsigned int i = 0; i < chars.size(); ++i) {
        char d = to_digit(i);
        char c = chars[i];
        INFO("i = " << i << "; d = " << d << "; c = " << c);
        REQUIRE( d == c );
    }
}

TEST_CASE("integral: cpow", "[integral]") {
    REQUIRE( cpow(2, 15) == 32768 );
    REQUIRE( cpow(3, 3) == 27 );
    REQUIRE( cpow(2, 8) == 256 );
    REQUIRE( cpow(2, 7) == 128 );
    REQUIRE( cpow(10, 0) == 1 );
    REQUIRE( cpow(10, 5) == 100000 );
}

TEST_CASE("integral: integral_lookup", "[integral]") {
    using lookup = integral_lookup<2, 4>;
    const lookup::table& t = lookup::instance;
    REQUIRE( lookup::pow == 16 );

    std::string_view v0( t.chars[0], t.chars[1] );
    std::string_view v9( t.chars[9], t.chars[10] );
    std::string_view v15( t.chars[14], t.chars[15] );

    REQUIRE( v0 == "0000" );
    REQUIRE( v9 == "1001" );
    REQUIRE( v15 == "1110" );
}

TEST_CASE("integral: unsigned_integral_abs", "[integral]") {
    unsigned_integral_abs_with_limits<char>();
    unsigned_integral_abs_with_limits<short>();
    unsigned_integral_abs_with_limits<int>();
    unsigned_integral_abs_with_limits<long>();
}

TEST_CASE("integral: unsigned_integral_length", "[integral]") {
    REQUIRE( unsigned_integral_length<unsigned char>(255, 16) == 2 );
    REQUIRE( unsigned_integral_length<unsigned int>(0, 10) == 1 );
    REQUIRE( unsigned_integral_length<unsigned int>(123, 10) == 3 );
    REQUIRE( unsigned_integral_length<unsigned int>(std::numeric_limits<unsigned int>::max(), 10) == 10 );
    REQUIRE( unsigned_integral_length<unsigned int>(std::numeric_limits<unsigned int>::max(), 2) == std::numeric_limits<unsigned int>::digits);
}

unsigned int unsigned_integral_length_simple(unsigned int value, unsigned int base) {
    unsigned int len = 1;
    for (; value >= base; value /= base, ++len);
    return len;
}

unsigned int unsigned_integral_length_pow10_conditions(unsigned int v) {
    // 4294967295
    // 1000000000
    // 100000000
    // 10000000
    // 1000000
    // 100000
    // 10000
    // 1000
    // 100
    // 10
    return v < 100000 
        ? v < 1000      ? (v < 10 ? 1 : v < 100 ? 2 : 3) : (v < 10000 ? 4 : 5)
        : v < 100000000 ? (v < 1000000 ? 6 : v < 10000000 ? 7 : 8) : (v < 1000000000 ? 9 : 10);
}

constexpr unsigned int pow10_table[] = {
    10,
    100,
    1000,
    10000,
    100000,
    1000000,
    10000000,
    100000000,
    1000000000
};

unsigned int unsigned_integral_length_pow10_binary_search(unsigned int value) {
    unsigned int l = 0, r = std::size(pow10_table), mid = std::size(pow10_table) >> 1;
    for (; l < r; mid = (l + r) >> 1) {
        if (value < pow10_table[mid]) r = mid; else l = mid + 1;
    }
    return mid + 1;
}

unsigned int unsigned_integral_length_pow10_std_binary_search(unsigned int value) {
    const unsigned int* p = std::upper_bound(pow10_table, pow10_table + std::size(pow10_table), value);
    return static_cast<unsigned int>(p - pow10_table) + 1;
}

unsigned int unsigned_integral_length_pow10_loop(unsigned int value) {
    for (unsigned int i = 0; i < std::size(pow10_table); ++i) {
        if (value < pow10_table[i]) return i + 1;
    }
    return std::size(pow10_table) + 1;
}

TEST_CASE("integral: unsigned_integral_length benchmark methods", "[integral]") {
    for (unsigned int i = 1; i <= 10; ++i) {
        unsigned int v = cpow(10, i - 1);
        REQUIRE(unsigned_integral_length(v, 10) == i);
        REQUIRE(unsigned_integral_length_simple(v, 10) == i);
        REQUIRE(unsigned_integral_length_pow10_conditions(v) == i);
        REQUIRE(unsigned_integral_length_pow10_binary_search(v) == i);
        REQUIRE(unsigned_integral_length_pow10_std_binary_search(v) == i);
        REQUIRE(unsigned_integral_length_pow10_loop(v) == i);
#ifdef _GLIBCXX_CHARCONV_H
        REQUIRE(std::__detail::__to_chars_len(v, 10) == i);
#endif
#ifdef _LIBCPP___CHARCONV_TO_CHARS_INTEGRAL_H
        REQUIRE(static_cast<unsigned int>(std::__1::__itoa::__traits<decltype(v)>::__width(v)) == i);
#endif
    }
}

TEST_CASE("integral: unsigned_integral_length benchmark", "[integral][!benchmark]") {
    using type = unsigned long;
    constexpr unsigned int base = 10;
    auto [pow, value] = GENERATE_COPY(base_gen<type>(base));
    DYNAMIC_SECTION("xlabel=# of digits;xtick=1;x=" << pow + 1 << ";base=" << base << ";value=" << value) {
        BENCHMARK("current") {
            return unsigned_integral_length(value, base);
        };
        BENCHMARK("simple") {
            return unsigned_integral_length_simple(value, base);
        };
        BENCHMARK("pow10 table: conditions") {
            return unsigned_integral_length_pow10_conditions(value);
        };
        BENCHMARK("pow10 table: binary_search") {
            return unsigned_integral_length_pow10_binary_search(value);
        };
        BENCHMARK("pow10 table: std::upper_bound") {
            return unsigned_integral_length_pow10_std_binary_search(value);
        };
        BENCHMARK("pow10 table: loop") {
            return unsigned_integral_length_pow10_loop(value);
        };
#ifdef _GLIBCXX_CHARCONV_H
        BENCHMARK("glibcxx") {
            return std::__detail::__to_chars_len(value, base);
        };
#endif
#ifdef _LIBCPP___CHARCONV_TO_CHARS_INTEGRAL_H
        BENCHMARK("libcpp") {
            return std::__1::__itoa::__traits<type>::__width(value);
        };
#endif
    }
}

TEST_CASE("integral: integral_length", "[integral]") {
    REQUIRE( integral_length<int>(-123, 10) == 4 );
    REQUIRE( integral_length<int>(123, 10) == 3 );
    REQUIRE( integral_length<int>(0, 10) == 1 );
    REQUIRE( integral_length<int>(std::numeric_limits<int>::min(), 10) == 11);
}

TEST_CASE("integral: unsigned_integral_lookup_chars", "[integral]") {
    char buf[128];
    unsigned int len = unsigned_integral_length<unsigned int>(12345, 10);
    unsigned_integral_lookup_chars<unsigned int, 10, 2>(buf, len, 12345);
    REQUIRE(std::string_view(buf, buf + len) == "12345");
    
    len = unsigned_integral_length<unsigned char>(255, 10);
    unsigned_integral_lookup_chars<unsigned char, 10, 2>(buf, len, 255);
    REQUIRE(std::string_view(buf, buf + len) == "255");

    len = unsigned_integral_length<unsigned char>(255, 2);
    unsigned_integral_lookup_chars<unsigned char, 2, 4>(buf, len, 255);
    REQUIRE(std::string_view(buf, buf + len) == "11111111");
}

TEST_CASE("integral: integral_chars", "[integral]") {
    char buf[128];
    integral_chars(buf, 4, -123, 10, false);
    REQUIRE(std::string_view(buf, buf + 4) == "-123");

    long v = std::numeric_limits<long>::min() + 1;
    
    unsigned int len = integral_length(v, 2);
    integral_chars(buf, len, v, 2, false);
    std::string_view integral_chars_str(buf, buf + len);
    REQUIRE(integral_chars_str == '-' + std::string(len - 1, '1'));

    char to_chars_buf[128];
    std::string_view to_chars_str(
        to_chars_buf, 
        std::to_chars(to_chars_buf, to_chars_buf + sizeof(to_chars_buf), v, 2).ptr);
    REQUIRE(to_chars_str == integral_chars_str);
}

template <std::unsigned_integral T>
void unsigned_integral_chars_digit_str(char* buf, unsigned int length, T value, unsigned int base) {
    static const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    for (char* ptr = buf + length; ptr != buf; value /= base) *--ptr = digits[value % base];
}

template <std::unsigned_integral T>
void unsigned_integral_chars_to_digit(char* buf, unsigned int length, T value, unsigned int base, bool upper) {
    for (char* ptr = buf + length; ptr != buf; value /= base) *--ptr = to_digit(value % base, upper);
}

TEST_CASE("integral: unsigned_integral_chars benchmark", "[integral][!benchmark]") {
    using type = unsigned long;
    constexpr unsigned int base = 10;
    const auto [pow, value] = GENERATE_COPY(base_gen<type>(base));
    DYNAMIC_SECTION("xlabel=# of digits;xtick=1;x=" << pow + 1 << ";base=" << base << ";value=" << value) {
        char buf[128];
        unsigned int len = unsigned_integral_length(value, base);
        BENCHMARK("chars_digit_str") {
            unsigned_integral_chars_digit_str(buf, len, value, base);
            return std::string_view(buf, buf + len);
        };
        BENCHMARK("chars_to_digit") {
            unsigned_integral_chars_to_digit(buf, len, value, base, false);
            return std::string_view(buf, buf + len);
        };
        BENCHMARK("lookup_chars") {
            unsigned_integral_lookup_chars<type, base, 2>(buf, len, value);
            return std::string_view(buf, buf + len);
        };
        BENCHMARK("std::to_string") {
            return std::to_string(value);
        };
        #if defined(__cpp_lib_format_ranges) || defined(__cpp_lib_format)
        BENCHMARK("std::format") {
            return std::string_view(buf, std::format_to(buf, "{}", value));
        };
        #endif
        BENCHMARK("std::to_chars") {
            // std::to_chars is highly optimized by using power & lookup tables
            // 4 chars for base = 2 
            // 2 chars for 8, 10, 16 bases
            std::to_chars_result result = std::to_chars(buf, buf + sizeof(buf), value, base);
            return std::string_view(buf, result.ptr);
        };
    }
}
