#pragma once

#include <ostream>
#include <ansipp/esc.hpp>
#include <ansipp/vec.hpp>

namespace ansipp {

/**
 * @brief checks whether stdin and stdout attached to terminal
 */
bool is_terminal();

vec get_terminal_size();

inline std::string hard_reset() { return esc + 'c'; }
inline std::string soft_reset() { return csi + "!p"; }

inline std::string enable_line_wrap() { return decset + "7h"; }
inline std::string disable_line_wrap() { return decset + "7l"; }

inline std::string enable_alternate_buffer() { return decset + "1049h"; }
inline std::string disable_alternate_buffer() { return decset + "1049l"; }

inline std::string enable_focus_reporting() { return decset + "1004h"; }
inline std::string disable_focus_reporting() { return decset + "1004l"; }

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
    return std::string(csi).append(1, static_cast<char>(mode)).append(1, static_cast<char>(target));
}

}