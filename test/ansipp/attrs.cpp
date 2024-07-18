#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark_all.hpp>

#include <iostream>
#include <ansipp/attrs.hpp>

using namespace ansipp;

TEST_CASE("attrs: format", "[attrs][!benchmark]") {
    rgb c = { 127, 0, 127 };
    BENCHMARK("impl") {
        return esc_str(attrs().fg(c));
    };
    BENCHMARK("format") {
        return std::format("\33[{};{};{};{};{}m", 38, 5, c.r, c.g, c.b);
    };
}

TEST_CASE("attrs: colors", "[attrs]") {
    REQUIRE( esc_str(attrs()) == "\33" "[m" );
    REQUIRE( esc_str(attrs().fg(RED).bg(BLUE).on(BOLD)) == "\33" "[31;44;1m" );
    REQUIRE( esc_str(attrs().off()) == "\33" "[0m" );
    REQUIRE( esc_str(attrs().off(BOLD)) == "\33" "[22m" );
    std::cout << attrs().off();
}

bool operator==(const rgb& a, const rgb& b) { return a.r == b.r && a.g == b.g && a.b == b.b; } 

TEST_CASE("rgb: lerp", "[rgb]") {
    const rgb a = { 0, 0, 0 }, b = { 255, 255, 255 };
    REQUIRE((rgb::lerp(a, b, 0.f) == a));
    REQUIRE((rgb::lerp(a, b, 1.f) == b));
    REQUIRE((rgb::lerp(a, b, 0.5f) == rgb { 127, 127, 127 }));
}