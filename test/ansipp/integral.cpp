#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark_all.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <limits>
#include <charconv>

#include <ansipp/integral.hpp>

using namespace ansipp;

template <typename T>
void unsigned_integral_abs_with_limits() {
    DYNAMIC_SECTION(typeid(T).name()) {
        using limits = std::numeric_limits<T>;
        const T min = limits::min();
        const T max = limits::max();
        REQUIRE( unsigned_integral_abs<T>(-1) == 1 );
        REQUIRE( unsigned_integral_abs<T>(max) == max );
    }
}

TEST_CASE("integral: unsigned_integral_abs", "[integral]") {
    unsigned_integral_abs_with_limits<char>();
    unsigned_integral_abs_with_limits<short>();
    unsigned_integral_abs_with_limits<int>();
    unsigned_integral_abs_with_limits<long>();
}

TEST_CASE("integral: unsigned_integral_length", "[integral]") {
    REQUIRE( unsigned_integral_length<unsigned int>(0, 10) == 1 );
    REQUIRE( unsigned_integral_length<unsigned int>(123, 10) == 3 );
    REQUIRE( unsigned_integral_length<unsigned int>(std::numeric_limits<unsigned int>::max(), 10) == 10 );
    REQUIRE( unsigned_integral_length<unsigned int>(std::numeric_limits<unsigned int>::max(), 2) == std::numeric_limits<unsigned int>::digits);
}

TEST_CASE("integral: integral_length", "[integral]") {
    REQUIRE( integral_length<int>(-123, 10) == 4 );
    REQUIRE( integral_length<int>(123, 10) == 3 );
    REQUIRE( integral_length<int>(0, 10) == 1 );
    REQUIRE( integral_length<int>(std::numeric_limits<int>::min(), 10) == 11);
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
void unsigned_integral_chars_single_loop(char* buf, unsigned int length, T value, unsigned int base, bool upper) {
    for (char* ptr = buf + length; ptr != buf; value /= base) *--ptr = to_digit(value % base, upper);
}

TEST_CASE("integral: unsigned_integral_chars benchmark", "[integral][!benchmark]") {
    unsigned long v = std::numeric_limits<unsigned long>::max();
    unsigned int base = GENERATE(2, 4, 8, 10, 12, 16);
    DYNAMIC_SECTION("base = " << base) {
        unsigned int len = unsigned_integral_length(v, base);

        char single_loop[128];
        BENCHMARK("unsigned_integral_chars_single_loop") {
            unsigned_integral_chars_single_loop(single_loop, len, v, base, false);
            return std::string_view(single_loop, single_loop + len);
        };

        char integral_chars_buf[128];
        BENCHMARK("unsigned_integral_chars") {
            unsigned_integral_chars(integral_chars_buf, len, v, base, false);
            return std::string_view(integral_chars_buf, integral_chars_buf + len);
        };

        char to_chars_buf[128];
        BENCHMARK("std_to_chars") {
            // std::to_chars is highly optimized by using power & lookup tables
            // 4 chars for base = 2 
            // 2 chars for 8, 10, 16 bases
            std::to_chars_result result = std::to_chars(to_chars_buf, to_chars_buf + sizeof(to_chars_buf), v, base);
            return std::string_view(to_chars_buf, result.ptr);
        };
    }
}
