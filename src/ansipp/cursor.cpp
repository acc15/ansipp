#include <ansipp/cursor.hpp>
#include <ansipp/io.hpp>

#include <string_view>
#include <charconv>

namespace ansipp {

cursor_position parse_cursor_position_escape(std::string_view v) {
    cursor_position p;
    if (v.size() < 6) {
        return p;
    }

    v.remove_prefix(2);
    v.remove_suffix(1);

    const auto sep_pos = v.find(';');
    if (sep_pos != std::string_view::npos) {
        std::string_view row_str = v.substr(0, sep_pos);
        std::string_view col_str = v.substr(sep_pos + 1);
        std::from_chars(row_str.begin(), row_str.end(), p.row);
        std::from_chars(col_str.begin(), col_str.end(), p.col);
    }
    return p;
}

cursor_position get_cursor_position() {
    terminal_write(request_position());
    std::string esc;
    return terminal_read(esc) ? parse_cursor_position_escape(esc) : cursor_position{};
}

}