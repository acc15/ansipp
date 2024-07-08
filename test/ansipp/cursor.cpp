#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark_all.hpp>

#include <ansipp/cursor.hpp>

using namespace ansipp;

bool operator==(const cursor_position& a, const cursor_position& b) { return a.row == b.row && a.col == b.col; }

TEST_CASE("cursor: parse_cursor_position_escape", "[cursor]") {
    REQUIRE(( parse_cursor_position_escape("\33" "[16;10R") == cursor_position { 16, 10 } ));
    REQUIRE(( parse_cursor_position_escape("\33" "[y;xR") == cursor_position { 0, 0 } ));
}

TEST_CASE("cursor: position escapes") {
    REQUIRE( move(CURSOR_UP, 5) == "\33" "[5A" );
    REQUIRE( move(5, 10) == "\33" "[5;10H" );
    REQUIRE( show_cursor() == "\33" "[?25h" );
    REQUIRE( hide_cursor() == "\33" "[?25l" );
    REQUIRE( store_cursor() == "\33" "7" );
    REQUIRE( restore_cursor() == "\33" "8" );
}

TEST_CASE("cursor: move format", "[cursor][!benchmark]") {
    const cursor_position p = { 5, 10 };

    std::string shared_str;
    shared_str.reserve(20);

    char printf_buf[20];

    BENCHMARK("format") {
        return std::format("\33[{};{}H", p.row, p.col);
    };
    BENCHMARK("append") {
        return std::string("\33[")
            .append(std::to_string(p.row)).append(1, ';')
            .append(std::to_string(p.col)).append(1, 'H');
    };
    BENCHMARK("append_shared") {
        return shared_str.assign("\33[")
            .append(std::to_string(p.row)).append(1, ';')
            .append(std::to_string(p.col)).append(1, 'H');
    };
    BENCHMARK("snprintf") {
        return snprintf(printf_buf, sizeof(printf_buf), "\33[%u;%uH", 
            static_cast<unsigned int>(p.row), static_cast<unsigned int>(p.col));
    };
    BENCHMARK("impl") {
        return move(p);
    };
}
