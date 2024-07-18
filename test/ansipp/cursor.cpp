#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark_all.hpp>
#include <ansipp/cursor.hpp>

using namespace ansipp;

TEST_CASE("cursor: parse_cursor_position_escape", "[cursor]") {
    REQUIRE(( parse_cursor_position_escape("\33" "[16;10R") == vec(10, 16) ));
    REQUIRE(( parse_cursor_position_escape("\33" "[y;xR") == vec() ));
}

TEST_CASE("cursor: position escapes") {
    REQUIRE( esc_str(move(CURSOR_UP, 0)) == "" );
    REQUIRE( esc_str(move(CURSOR_UP, 5)) == "\33" "[5A" );
    REQUIRE( esc_str(move_abs(10, 5)) == "\33" "[5;10H" );
    REQUIRE( cursor_visibility.on() == "\33" "[?25h" );
    REQUIRE( cursor_visibility.off() == "\33" "[?25l" );
    REQUIRE( store_cursor == "\33" "7" );
    REQUIRE( restore_cursor == "\33" "8" );
}

