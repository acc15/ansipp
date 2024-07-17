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

const decset_mode line_wrap = 7;
const decset_mode alternate_buffer = 1049;
const decset_mode focus_reporting = 1004;

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