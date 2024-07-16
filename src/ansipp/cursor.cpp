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
    std::string rq_cur = request_cursor(); 
    if (auto w = terminal_write(rq_cur); static_cast<std::streamsize>(rq_cur.size()) != w) { return vec{}; }
    
    char buf[20];
    std::string_view rd;
    return terminal_read(buf, rd) ? parse_cursor_position_escape(rd) : vec{};
}

std::string move(move_mode mode, unsigned int value) {
    if (mode == CURSOR_TO_COLUMN && value < 2) return "\r";
    return value > 0 
        ? std::string(csi).append(std::to_string(value)).append(1, static_cast<char>(mode))
        : std::string();
}

std::string move_abs(int x, int y) {
    return std::string(csi).append(std::to_string(y)).append(1, ';').append(std::to_string(x)).append(1, 'H');
}

std::string move_x(int x) {
    return x >= 0 ? move(CURSOR_RIGHT, static_cast<unsigned int>(x)) : move(CURSOR_LEFT, static_cast<unsigned int>(-x));
}

std::string move_y(int y) {
    return y >= 0 ? move(CURSOR_DOWN, static_cast<unsigned int>(y)) : move(CURSOR_UP, static_cast<unsigned int>(-y));
}

std::string move_rel(int x, int y) { 
    return move_y(y) + move_x(x); 
}

}