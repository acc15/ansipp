#include <catch2/catch_test_macros.hpp>

#include <ansipp\vec.hpp>

using namespace ansipp;

TEST_CASE("vec: aggregate initialization", "[vec]") {
    vec v = { 5, 4 };
    REQUIRE(v.x == 5);
    REQUIRE(v.y == 4);
}

TEST_CASE("vec: equality", "[vec]") {
    REQUIRE( vec { 5, 4 } == vec { 5, 4 } );
    REQUIRE( vec { 5, 4 } != vec { 2, 4 } );
}

TEST_CASE("vec: add", "[vec]") {
    vec v1 = { 5, 4 };
    vec v2 = { 2, 4 };
    vec sum = v1 + v2;
    REQUIRE(sum.x == 7);
    REQUIRE(sum.y == 8);
}

TEST_CASE("vec: int assignment", "[vec]") {
    vec v = 11;
    v = 9;
    REQUIRE(v == vec { 9, 9 });
}

TEST_CASE("vec: int initialization", "[vec]") {
    vec v = 11;
    REQUIRE(v == vec { 11, 11 });
}

TEST_CASE("vec: int initialization in increment", "[vec]") {
    vec v = 11;
    v += 12;
    REQUIRE(v == vec { 23, 23 });
}