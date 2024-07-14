#pragma once

#include <ostream>
#include <ansipp/vec.hpp>

namespace ansipp {

/**
 * @brief checks whether stdin and stdout attached to terminal
 */
bool is_terminal();

vec get_terminal_size();

inline std::string store_screen() { return "\33" "[?47h"; }
inline std::string restore_screen() { return "\33" "[?47l"; }

inline std::string enable_line_wrap() { return "\33" "[?7h"; }
inline std::string disable_line_wrap() { return "\33" "[?7l"; }

inline std::string enable_alternate_buffer() { return "\33" "[?1049h"; }
inline std::string disable_alternate_buffer() { return "\33" "[?1049l"; }

enum erase_target: char {
    SCREEN = 'J',
    LINE = 'K'
};

enum erase_mode: char {
    TO_END = '0',
    TO_BEGIN = '1',
    ALL = '2'
};

inline std::string erase(erase_target target, erase_mode mode) {
    return std::string("\33" "[")
        .append(1, static_cast<char>(mode))
        .append(1, static_cast<char>(target));
}



}