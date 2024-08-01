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

const std::string hard_reset = esc + 'c'; 
const std::string soft_reset = csi + "!p";

constexpr decset_mode line_wrap = 7;
constexpr decset_mode alternate_buffer = 1049;
constexpr decset_mode focus_reporting = 1004;

enum erase_target: char {
    SCREEN = 'J',
    LINE = 'K'
};

enum erase_mode: char {
    TO_END = '0',
    TO_BEGIN = '1',
    ALL = '2'
};

struct erase {
    erase_target target;
    erase_mode mode;
    erase(erase_target target, erase_mode mode): target(target), mode(mode) {}
};
template <typename Stream>
Stream& operator<<(Stream& s, erase esc) {
    return s << csi << static_cast<char>(esc.mode) << static_cast<char>(esc.target);
}

}