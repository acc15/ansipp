#include <catch2/catch_test_macros.hpp>

#include <ansipp/charbuf.hpp>
#include <limits>

using namespace ansipp;

TEST_CASE("empty to string", "[charbuf]") {
    charbuf cb;
    REQUIRE( cb.str() == "" );
    REQUIRE( cb.view() == "" );
}

TEST_CASE("string", "[charbuf]") {
    charbuf cb;
    std::string str = "123";
    cb << str;
    REQUIRE( cb.view() == "123" );
}

TEST_CASE("integral", "[charbuf]") {
    charbuf cb;
    unsigned int uint_max = std::numeric_limits<unsigned int>::max();
    REQUIRE( (cb.clear() << -123).view() == "-123" );
    REQUIRE( (cb.clear() << uint_max).view() == std::to_string(uint_max) );
}

TEST_CASE("bool", "[charbuf]") {
    charbuf cb;
    REQUIRE( (cb.clear() << false).view() == "0" );
    REQUIRE( (cb.clear() << true).view() == "1" );
}