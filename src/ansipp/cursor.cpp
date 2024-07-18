#include <ansipp/cursor.hpp>
#include <ansipp/io.hpp>

#include <string_view>
#include <charconv>

namespace ansipp {

vec parse_cursor_position_escape(std::string_view v) {
    vec p = {};
    if (v.size() < 6) {
        return p;
    }

    v.remove_prefix(2);
    v.remove_suffix(1);

    const auto sep_pos = v.find(';');
    if (sep_pos != std::string_view::npos) {
        std::string_view row_str = v.substr(0, sep_pos);
        std::string_view col_str = v.substr(sep_pos + 1);
        std::from_chars(row_str.data(), row_str.data() + row_str.size(), p.y);
        std::from_chars(col_str.data(), col_str.data() + col_str.size(), p.x);
    }
    return p;
}

vec get_cursor_position() {
    if (auto w = terminal_write(request_cursor); static_cast<std::streamsize>(request_cursor.size()) != w) { return vec{}; }
    
    char buf[20];
    std::string_view rd;
    return terminal_read(buf, rd) ? parse_cursor_position_escape(rd) : vec{};
}

}