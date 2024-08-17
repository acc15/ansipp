#include <ansipp/input.hpp>

#include <unordered_map>

namespace ansipp {

std::unordered_map<std::string_view, input_sequence_type> escapes = {
    {"\x1b[A", KEY_UP},
    {"\x1b[B", KEY_DOWN},
    {"\x1b[C", KEY_RIGHT},
    {"\x1b[D", KEY_LEFT},
};

std::vector<input_sequence> parse_escapes([[maybe_unused]] std::string_view& buffer_view) {
    std::vector<input_sequence> seq;
    return seq;
}

}