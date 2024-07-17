#pragma once

#include <string>
#include <ostream>
#include <ansipp/vec.hpp>
#include <ansipp/esc.hpp>

namespace ansipp {

vec parse_cursor_position_escape(std::string_view v);
vec get_cursor_position();

const decset_mode cursor_visibility = 25;

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

inline std::string store_cursor() { return esc + "7"; }
inline std::string restore_cursor() { return esc + "8"; }
inline std::string request_cursor() { return csi + "6n"; }

enum cursor_shape: char {
    SHAPE_DEFAULT = '0',
    SHAPE_BLINK_BLOCK = '1',
    SHAPE_STEADY_BLOCK = '2',
    SHAPE_BLINK_UNDERLINE = '3',
    SHAPE_STEADY_UNDERLINE = '4',
    SHAPE_BLINK_BAR = '5',
    SHAPE_STEADY_BAR = '6'
};

inline std::string change_cursor_shape(cursor_shape shape) { 
    return std::string(csi).append(1, static_cast<char>(shape)).append(" q");
}

}