#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark_all.hpp>

#include <iostream>

#include <ansipp.hpp>

using namespace ansipp;

TEST_CASE("format vs append", "[escape][!benchmark]") {
    const cursor_position p = { 5, 10 };
    BENCHMARK("format") {
        return std::format("\33[{};{}H", p.row, p.col);
    };
    BENCHMARK("append") {
        return std::string("\33[")
            .append(std::to_string(p.row)).append(1, ';')
            .append(std::to_string(p.col)).append(1, 'H');
    };
}

TEST_CASE("position escapes") {
    REQUIRE( move(UP, 5) == "\33[5A" );
    REQUIRE( move({ 5, 10 }) == "\33[5;10H" );
    REQUIRE( show_cursor() == "\33[?25h" );
    REQUIRE( hide_cursor() == "\33[?25l" );
    REQUIRE( save_position() == "\0337" );
    REQUIRE( restore_position() == "\0338" );
}

TEST_CASE("colors") {
    REQUIRE( attrs().fg(RED).bg(BLUE).on(BOLD).str() == "\033[31;44;1m" );
    REQUIRE( attrs().off().str() == "\033[0m" );
    REQUIRE( attrs().off(BOLD).str() == "\033[22m" );
    std::cout << attrs().off();
}