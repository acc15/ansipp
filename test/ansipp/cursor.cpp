#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark_all.hpp>

#include <ansipp/cursor.hpp>

using namespace ansipp;

TEST_CASE("cursor: parse_cursor_position_escape", "[cursor]") {
    REQUIRE(( parse_cursor_position_escape("\33" "[16;10R") == vec(10, 16) ));
    REQUIRE(( parse_cursor_position_escape("\33" "[y;xR") == vec() ));
}

TEST_CASE("cursor: position escapes") {
    REQUIRE( move(CURSOR_UP, 0) == "" );
    REQUIRE( move(CURSOR_UP, 5) == "\33" "[5A" );
    REQUIRE( move_abs(10, 5) == "\33" "[5;10H" );
    REQUIRE( cursor_visibility.on() == "\33" "[?25h" );
    REQUIRE( cursor_visibility.off() == "\33" "[?25l" );
    REQUIRE( store_cursor() == "\33" "7" );
    REQUIRE( restore_cursor() == "\33" "8" );
}

TEST_CASE("cursor: move format", "[cursor][!benchmark]") {
    const vec p = { 5, 10 };

    std::string shared_str;
    shared_str.reserve(20);

    char printf_buf[20];

    BENCHMARK("format") {
        return std::format("{}{};{}H", csi, p.y, p.x);
    };
    BENCHMARK("append") {
        return std::string(csi)
            .append(std::to_string(p.y)).append(1, ';')
            .append(std::to_string(p.x)).append(1, 'H');
    };
    BENCHMARK("append_shared") {
        return shared_str.assign(csi)
            .append(std::to_string(p.y)).append(1, ';')
            .append(std::to_string(p.x)).append(1, 'H');
    };
    BENCHMARK("stringstream") {
        std::stringstream ss;
        ss << csi << p.y << ';' << p.x << 'H';
        return ss.view();
    };
    BENCHMARK("snprintf") {
        return snprintf(printf_buf, sizeof(printf_buf), "%s%u;%uH", 
            csi.c_str(), static_cast<unsigned int>(p.y), static_cast<unsigned int>(p.x));
    };
    BENCHMARK("impl") {
        return move_abs(p);
    };
}
