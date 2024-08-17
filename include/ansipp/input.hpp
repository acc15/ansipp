#pragma once

#include <string_view>
#include <vector>

namespace ansipp {

enum class input_type {
    UNKNOWN,
    CHAR,
    UP,
    DOWN,
    LEFT,
    RIGHT
};

enum class input_modifiers {
    SHIFT,
    ALT,
    CTRL,
    SUPER,
    HYPER,
    META,
    CAPS_LOCK,
    NUM_LOCK,
};

struct input_event {
    input_modifiers modifiers;
    input_type type;
    std::string_view view;
};

void parse_input_escapes(std::string_view& input, std::vector<input_event>& events, bool consume_all);

}