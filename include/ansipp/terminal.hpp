#pragma once

#include <ostream>

namespace ansipp {

/**
 * @brief checks whether stdin and stdout attached to terminal
 */
bool is_terminal();

struct terminal_dimension {
    unsigned short rows;
    unsigned short cols;
};
inline std::ostream& operator<<(std::ostream& o, const terminal_dimension& d) {
    return o << d.cols << "x" << d.rows;
}
terminal_dimension get_terminal_dimension();

inline std::string save_screen() { return "\33" "[?47h"; }
inline std::string restore_screen() { return "\33" "[?47l"; }

inline std::string enable_alternative_buffer() { return "\33" "[?1049h"; }
inline std::string disable_alternative_buffer() { return "\33" "[?1049l"; }

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