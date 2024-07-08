#pragma once

#include <string>
#include <ostream>

namespace ansipp {

struct cursor_position {
    unsigned short row;
    unsigned short col;
};
inline std::ostream& operator<<(std::ostream& o, const cursor_position& p) {
    return o << p.col << "," << p.row;
}

cursor_position parse_cursor_position_escape(std::string_view v);
cursor_position get_cursor_position();

inline std::string show_cursor() { return "\33" "[?25h"; }
inline std::string hide_cursor() { return "\33" "[?25l"; }

enum move_mode: char {
    CURSOR_UP = 'A', 
    CURSOR_DOWN = 'B', 
    CURSOR_RIGHT = 'C', 
    CURSOR_LEFT = 'D', 
    NEXT_LINE = 'E', 
    PREV_LINE = 'F', 
    CURSOR_TO_COLUMN = 'G',
    SCROLL_UP = 'S',
    SCROLL_DOWN = 'T'
};

inline std::string move(move_mode mode, unsigned int value = 1) {
    return std::string("\33" "[").append(std::to_string(value)).append(1, static_cast<char>(mode));
}

inline std::string move(unsigned short row, unsigned short col) {
    return std::string("\33" "[")
        .append(std::to_string(row)).append(1, ';')
        .append(std::to_string(col)).append(1, 'H');
}

inline std::string move(const cursor_position& p) { return move(p.row, p.col); }

inline std::string save_position() { return "\33" "7"; }
inline std::string restore_position() { return "\33" "8"; }
inline std::string request_position() { return "\33" "[6n"; }

}