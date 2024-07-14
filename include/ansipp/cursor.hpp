#pragma once

#include <string>
#include <ostream>
#include <ansipp/vec.hpp>

namespace ansipp {



vec parse_cursor_position_escape(std::string_view v);
vec get_cursor_position();

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
std::string move_abs(int x, int y);
std::string move_rel(int x, int y);

inline std::string move_abs(const vec& v) { return move_abs(v.x, v.y); }
inline std::string move_rel(const vec& v) { return move_rel(v.x, v.y); }

inline std::string store_cursor() { return "\33" "7"; }
inline std::string restore_cursor() { return "\33" "8"; }
inline std::string request_cursor() { return "\33" "[6n"; }

}