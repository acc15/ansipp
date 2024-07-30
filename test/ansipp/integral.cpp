#include <version>
#include <limits>
#include <charconv>
#include <algorithm>
#include <cstdlib>
#include <iostream>

#if defined(__cpp_lib_format_ranges) || defined(__cpp_lib_format)
#   include <format>
#endif

#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark_all.hpp>
#include <catch2/generators/catch_generators_all.hpp>

#include <ansipp/integral.hpp>
#include "pow_gen.hpp"

using namespace ansipp;

using pt10 = ipow_lookup<10>;

TEST_CASE("integral: unsigned comparisons", "[integral]") {
    unsigned char v = 255;

    unsigned int ui = 256;
    REQUIRE( v < ui );
    REQUIRE( ui >= v );
}

TEST_CASE("integral: digit", "[integral]") {
    std::string_view chars = "0123456789abcdefghijklmnopqrstuvwxyz";
    for (unsigned int i = 0; i < chars.size(); ++i) {
        char d = digit(i);
        char c = chars[i];
        INFO("i = " << i << "; d = " << d << "; c = " << c);
        REQUIRE( d == c );
    }
}

TEST_CASE("integral: cpow", "[integral]") {
    REQUIRE( ipow(2, 15) == 32768 );
    REQUIRE( ipow(3, 3) == 27 );
    REQUIRE( ipow(2, 8) == 256 );
    REQUIRE( ipow(2, 7) == 128 );
    REQUIRE( ipow(10, 0) == 1 );
    REQUIRE( ipow(10, 5) == 100000 );
}

TEST_CASE("integral: imaxpow", "[integral]") {
    const auto mp2 = imaxpow<unsigned char>(2);
    REQUIRE( mp2.first == 7 );
    REQUIRE( mp2.second == 128U );

    const auto mp10 = imaxpow<unsigned int>(10);
    REQUIRE( mp10.first == 9 );
    REQUIRE( mp10.second == 1000000000U );

    const auto mpl10 = imaxpow<unsigned long long>(10);
    REQUIRE( mpl10.first == 19 );
    REQUIRE( mpl10.second == 10000000000000000000UL );
}

TEST_CASE("integral: ipow_lookup", "[integral]") {
    REQUIRE( pt10::data.pow[0] == 10 );
    REQUIRE( pt10::data.pow[4] == 100000 );
}

TEST_CASE("integral: ipow_lookup print", "[.print][integral]") {
    std::cout << "table size: " << pt10::size << std::endl;
    for (unsigned int i = 0; i < pt10::size; ++i) {
        std::cout << i << " = " << pt10::data.pow[i] << std::endl;
    }
}

TEST_CASE("integral: ichars_lookup", "[integral]") {
    constexpr unsigned int digits = 4;
    using lookup = ichars_lookup<2, digits>;
    const lookup::table_data& t = lookup::data;
    REQUIRE( lookup::pow == 16 );
    REQUIRE( std::string_view(t.chars[0], t.chars[0] + digits) == "0000" );
    REQUIRE( std::string_view(t.chars[9], t.chars[9] + digits) == "1001" );
    REQUIRE( std::string_view(t.chars[14], t.chars[14] + digits) == "1110" );
}

TEST_CASE("integral: iabs", "[integral]") {
    std::intmax_t v = std::numeric_limits<std::intmax_t>::max();
    REQUIRE( iabs(v) == static_cast<std::uintmax_t>(v) );
    REQUIRE( iabs(-1) == 1 );
}

TEST_CASE("integral: ulen print", "[.print][integral]") {
    for (unsigned int i = 0; i < 1024; ++i) {
        std::cout << i 
            << " b2=" << ulen(i, 2) 
            << " b8=" << ulen(i, 8) 
            << " b10=" << ulen(i, 10) 
            << " b16=" << ulen(i, 16) 
            << std::endl;
    }
}

unsigned int ulen_simple(unsigned int value, unsigned int base) {
    unsigned int len = 1;
    for (; value >= base; value /= base, ++len);
    return len;
}

unsigned int ulen_pow10_conditions(unsigned int v) {
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

unsigned int ulen_pow10_binary_search(unsigned int value) {
    unsigned int l = 0, r = ipow_lookup<10>::size, mid = ipow_lookup<10>::size >> 1;
    for (; l < r; mid = (l + r) >> 1) {
        if (value < ipow_lookup<10>::data.pow[mid]) r = mid; else l = mid + 1;
    }
    return mid + 1;
}

unsigned int ulen_pow10_std_upper_bound(unsigned int value) {
    const auto& pt = pt10::data;
    const std::uintmax_t* p = std::upper_bound(pt.pow, pt.pow + pt10::size, value);
    return static_cast<unsigned int>(p - pt.pow) + 1;
}

unsigned int ulen_pow10_loop(unsigned int value) {
    for (unsigned int i = 0; i < pt10::size; ++i) {
        if (value < pt10::data.pow[i]) return i + 1;
    }
    return pt10::size + 1;
}

TEST_CASE("integral: ulen", "[integral]") {
    constexpr unsigned int base = 10;
    
    REQUIRE( ulen(255, 16) == 2 );
    REQUIRE( ulen(0, 10) == 1 );
    REQUIRE( ulen(123, 10) == 3 );
    REQUIRE( ulen(64, 10) == 2 );
    REQUIRE( ulen(512, 10) == 3 );
    REQUIRE( ulen(std::numeric_limits<unsigned int>::max(), 10) == 10 );
    REQUIRE( ulen(std::numeric_limits<unsigned int>::max(), 2) == std::numeric_limits<unsigned int>::digits);

    auto [d, v] = GENERATE(pow_gen<unsigned int>::wrap(base));
    CHECK(ulen(v, 10) == d);
    CHECK(ulen_simple(v, 10) == d);
    CHECK(ulen_pow10_conditions(v) == d);
    CHECK(ulen_pow10_binary_search(v) == d);
    CHECK(ulen_pow10_std_upper_bound(v) == d);
    CHECK(ulen_pow10_loop(v) == d);
#ifdef _GLIBCXX_CHARCONV_H
    CHECK(std::__detail::__to_chars_len(v, 10) == d);
#endif
#ifdef _LIBCPP___CHARCONV_TO_CHARS_INTEGRAL_H
    CHECK(static_cast<unsigned int>(std::__1::__itoa::__traits<decltype(v)>::__width(v)) == d);
#endif
}

TEST_CASE("integral: ulen benchmark", "[integral][!benchmark]") {
    constexpr unsigned int base = 10;
    auto [digits, value] = GENERATE(pow_gen<unsigned int>::wrap(base));
    DYNAMIC_SECTION("xlabel=# of digits;xtick=1;x=" << digits << ";base=" << base << ";value=" << value) {
        BENCHMARK("current") {
            return ulen(value, base);
        };
        BENCHMARK("simple") {
            return ulen_simple(value, base);
        };
        BENCHMARK("pow10 table: conditions") {
            return ulen_pow10_conditions(value);
        };
        BENCHMARK("pow10 table: binary_search") {
            return ulen_pow10_binary_search(value);
        };
        BENCHMARK("pow10 table: std::upper_bound") {
            return ulen_pow10_std_upper_bound(value);
        };
        BENCHMARK("pow10 table: loop") {
            return ulen_pow10_loop(value);
        };
#ifdef _GLIBCXX_CHARCONV_H
        BENCHMARK("glibcxx") {
            return std::__detail::__to_chars_len(value, base);
        };
#endif
#ifdef _LIBCPP___CHARCONV_TO_CHARS_INTEGRAL_H
        BENCHMARK("libcpp") {
            return std::__1::__itoa::__traits<decltype(value)>::__width(value);
        };
#endif
    }
}

TEST_CASE("integral: ilen", "[integral]") {
    REQUIRE( ilen(-123, 10) == 4 );
    REQUIRE( ilen(123, 10) == 3 );
    REQUIRE( ilen(0, 10) == 1 );
    REQUIRE( ilen(std::numeric_limits<int>::min(), 10) == 11);
}

TEST_CASE("integral: ulookupchars", "[integral]") {
    char buf[128];
    unsigned int len = ulen(12345, 10);
    ulookupchars<10, 2>(buf, len, 12345);
    REQUIRE(std::string_view(buf, buf + len) == "12345");
    
    len = ulen(255, 10);
    ulookupchars<10, 2>(buf, len, 255);
    REQUIRE(std::string_view(buf, buf + len) == "255");

    len = ulen(255, 2);
    ulookupchars<2, 4>(buf, len, 255);
    REQUIRE(std::string_view(buf, buf + len) == "11111111");
}

TEST_CASE("integral: ichars", "[integral]") {
    char buf[128];
    ichars(buf, 4, -123, 10, false);
    REQUIRE(std::string_view(buf, buf + 4) == "-123");

    long v = std::numeric_limits<long>::min() + 1;
    
    unsigned int len = ilen(v, 2);
    ichars(buf, len, v, 2, false);
    std::string_view integral_chars_str(buf, buf + len);
    REQUIRE(integral_chars_str == '-' + std::string(len - 1, '1'));

    char to_chars_buf[128];
    std::string_view to_chars_str(
        to_chars_buf, 
        std::to_chars(to_chars_buf, to_chars_buf + sizeof(to_chars_buf), v, 2).ptr);
    REQUIRE(to_chars_str == integral_chars_str);
}

template <std::unsigned_integral T>
void uchars_digit_str(char* buf, unsigned int length, T value, unsigned int base) {
    static const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    for (char* ptr = buf + length; ptr != buf; value /= base) *--ptr = digits[value % base];
}

template <std::unsigned_integral T>
void uchars_digit(char* buf, unsigned int length, T value, unsigned int base, bool upper) {
    for (char* ptr = buf + length; ptr != buf; value /= base) *--ptr = digit(value % base, upper);
}

TEST_CASE("integral: uchars benchmark", "[integral][!benchmark]") {
    constexpr unsigned int base = 10;
    const auto [digits, value] = GENERATE(pow_gen<unsigned long>::wrap(10));
    DYNAMIC_SECTION("xlabel=# of digits;xtick=1;x=" << digits << ";base=" << base << ";value=" << value) {
        char buf[128];
        BENCHMARK("current") {
            unsigned int len = ulen(value, base);
            uchars(buf, len, value, base, false);
            return std::string_view(buf, buf + len);
        };
        BENCHMARK("uchars_digit_str") {
            unsigned int len = ulen(value, base);
            uchars_digit_str(buf, len, value, base);
            return std::string_view(buf, buf + len);
        };
        BENCHMARK("uchars_digit") {
            unsigned int len = ulen(value, base);
            uchars_digit(buf, len, value, base, false);
            return std::string_view(buf, buf + len);
        };
        BENCHMARK("ulookupchars") {
            unsigned int len = ulen(value, base);
            ulookupchars<base, 2>(buf, len, value);
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
