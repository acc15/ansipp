#include <catch2/catch_test_macros.hpp>

#include <ansipp.hpp>

using namespace ansipp;

constexpr std::string str = erase(erase_target::LINE, erase_mode::TO_BEGIN);

TEST_CASE("position", "[ansipp]") {
    
    REQUIRE( move(move_mode::UP, 5) == "\33[5A" );
    REQUIRE( move(cursor_position { 5, 10 }) == "\33[5;10H" );
    REQUIRE( show_cursor() == "\33[?25h" );
    REQUIRE( hide_cursor() == "\33[?25l" );
    REQUIRE( save_cursor_position() == "\0337" );
    REQUIRE( restore_cursor_position() == "\0338" );

}

TEST_CASE("attr", "[ansipp]") {
    
    // attrs(attr::BOLD, attr::FG | attr::BRIGHT | color::RED, )

    REQUIRE( move(move_mode::UP, 5) == "\33[5A" );
    REQUIRE( move(cursor_position { 5, 10 }) == "\33[5;10H" );
    REQUIRE( show_cursor() == "\33[?25h" );
    REQUIRE( hide_cursor() == "\33[?25l" );
    REQUIRE( save_cursor_position() == "\0337" );
    REQUIRE( restore_cursor_position() == "\0338" );

}