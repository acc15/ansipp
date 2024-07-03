#include <catch2/catch_test_macros.hpp>

#include <ansipp.hpp>

using namespace ansipp;

TEST_CASE("ansi", "[ansipp]") {
    
    REQUIRE( move(move_mode::UP, 5) == "\33[5A" );
    REQUIRE( move(cursor_position { 5, 10 }) == "\33[5;10H" );
    REQUIRE( show_cursor() == "\33[?25h" );
    REQUIRE( hide_cursor() == "\33[?25l" );
    REQUIRE( save_cursor_position() == "\0337" );
    REQUIRE( restore_cursor_position() == "\0338" );

}