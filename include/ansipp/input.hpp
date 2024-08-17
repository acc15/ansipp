#pragma once

#include <string_view>
#include <vector>

namespace ansipp {

enum input_sequence_type {
    UNKNOWN,
    CHAR,
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT
};

struct input_sequence {
    input_sequence_type type;
    std::string_view view;
};

std::vector<input_sequence> parse_escapes(std::string_view& buffer_view);

}