#include <catch2/catch_test_macros.hpp>

#include <ansipp/charbuf.hpp>
#include <limits>
#include <numbers>

using namespace ansipp;

TEST_CASE("charbuf: empty to string", "[charbuf]") {
    charbuf cb;
    REQUIRE( cb.str() == "" );
    REQUIRE( cb.view() == "" );
}

TEST_CASE("charbuf: string", "[charbuf]") {
    charbuf cb;
    std::string str = "123";
    cb << str;
    REQUIRE( cb.view() == "123" );
}

TEST_CASE("charbuf: integral", "[charbuf]") {
    charbuf cb;
    unsigned int uint_max = std::numeric_limits<unsigned int>::max();
    REQUIRE( (cb.reset() << -123).view() == "-123" );
    REQUIRE( (cb.reset() << uint_max).view() == std::to_string(uint_max) );
}

TEST_CASE("charbuf: floating", "[charbuf]") {
    charbuf cb;
    cb << std::numbers::pi;
    REQUIRE(cb.flush() == "3.141592653589793");
    cb << floating_format(std::numbers::pi, std::chars_format::fixed, 5);
    REQUIRE(cb.flush() == "3.14159");
    cb << floating_format(std::numbers::pi, std::chars_format::hex, 2);
    REQUIRE(cb.flush() == "1.92p+1");
    cb << floating_format(std::numbers::pi, std::chars_format::fixed, 0);
    REQUIRE(cb.flush() == "3");
    cb << floating_format(std::numbers::pi, std::chars_format::scientific, 2);
    REQUIRE(cb.flush() == "3.14e+00");
    cb << floating_format(std::numbers::pi, std::chars_format::fixed, 64);
    REQUIRE(cb.flush() == "3.1415926535897931159979634685441851615905761718750000000000000000");
}

TEST_CASE("charbuf: bool", "[charbuf]") {
    charbuf cb;
    REQUIRE( (cb.reset() << false).view() == "0" );
    REQUIRE( (cb.reset() << true).view() == "1" );
}