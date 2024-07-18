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

struct move_esc { move_mode mode; unsigned int value; };
template <typename Stream>
Stream& operator<<(Stream& s, const move_esc& op) {
    if (op.mode == CURSOR_TO_COLUMN && op.value < 2) return s << '\r';
    if (op.value > 0) s << csi << op.value << static_cast<char>(op.mode);
    return s;
}
inline move_esc move(move_mode mode, unsigned int value = 1) { return move_esc { mode, value }; }


struct move_rel_esc { int x, y; };
template <typename Stream>
Stream& operator<<(Stream& s, const move_rel_esc& op) {
    s << (op.x < 0 
        ? move(CURSOR_LEFT, static_cast<unsigned int>(-op.x)) 
        : move(CURSOR_RIGHT, static_cast<unsigned int>(op.x)));
    s << (op.y < 0 
        ? move(CURSOR_UP, static_cast<unsigned int>(-op.y))
        : move(CURSOR_DOWN, static_cast<unsigned int>(op.y)));
    return s;
}
inline move_rel_esc move_rel(int x, int y) { return move_rel_esc { x, y }; }
inline move_rel_esc move_rel(const vec& v) { return move_rel_esc { v.x, v.y }; }


struct move_abs_esc { int x, y; };
template <typename Stream>
Stream& operator<<(Stream& s, const move_abs_esc& op) { return s << csi << op.y << ';' << op.x << 'H'; }
inline move_abs_esc move_abs(int x, int y) { return move_abs_esc { x, y }; }
inline move_abs_esc move_abs(const vec& v) { return move_abs_esc { v.x, v.y }; }


const std::string store_cursor = esc + "7";
const std::string restore_cursor = esc + "8";
const std::string request_cursor = csi + "6n";

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