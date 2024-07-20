#include <catch2/catch_test_macros.hpp>

#include <ansipp/integral.hpp>

#include <limits>

using namespace ansipp;

template <typename T>
void unsigned_abs_with_limits() {
    using limits = std::numeric_limits<T>;
    const T min = limits::min();
    const T max = limits::max();
    REQUIRE( unsigned_abs<T>(min) == -static_cast<long int>(min) );
    REQUIRE( unsigned_abs<T>(-1) == 1 );
    REQUIRE( unsigned_abs<T>(max) == max );
}

TEST_CASE("integral: unsigned_abs", "[integral]") {
    unsigned_abs_with_limits<char>();
    unsigned_abs_with_limits<short>();
    unsigned_abs_with_limits<int>();
    unsigned_abs_with_limits<long>();
}

TEST_CASE("integral: unsigned_digit_count", "[integral]") {
    REQUIRE( unsigned_digit_count<unsigned int>(0, 10) == 1 );
    REQUIRE( unsigned_digit_count<unsigned int>(123, 10) == 3 );
    REQUIRE( unsigned_digit_count<unsigned int>(std::numeric_limits<unsigned int>::max(), 10) == 10 );
    REQUIRE( unsigned_digit_count<unsigned int>(std::numeric_limits<unsigned int>::max(), 2) == CHAR_BIT * sizeof(unsigned int));
}