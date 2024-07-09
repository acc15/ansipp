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
    
    /**
     * @brief moves cursor to N lines up
     */
    CURSOR_UP = 'A', 
    
    /**
     * @brief moves cursor to N lines down
     */
    CURSOR_DOWN = 'B', 
    
    /**
     * @brief moves cursor to N cols right
     */
    CURSOR_RIGHT = 'C', 
    
    /**
     * @brief moves cursor to N cols left
     */
    CURSOR_LEFT = 'D', 
    
    /**
     * @brief moves cursor to N lines down and to 1st column
     */
    CURSOR_DOWN_START = 'E', 
    
    /**
     * @brief moves cursor to N lines up and to 1st column
     */
    CURSOR_UP_START = 'F', 
    
    /**
     * @brief moves cursor to specified column (line number isn't changed)
     */
    CURSOR_TO_COLUMN = 'G',
    
    /**
     * @brief scrolls up by N lines
     */
    SCROLL_UP = 'S',
    
    /**
     * @brief scrolls down by N lines
     */
    SCROLL_DOWN = 'T'
};

std::string move(move_mode mode, unsigned int value = 1);
std::string move(unsigned short row, unsigned short col);
inline std::string move(const cursor_position& p) { return move(p.row, p.col); }

std::string move_x(int x);
std::string move_y(int y);
std::string move_xy(int x, int y);

inline std::string store_cursor() { return "\33" "7"; }
inline std::string restore_cursor() { return "\33" "8"; }
inline std::string request_cursor() { return "\33" "[6n"; }

}