#include <ansipp/input.hpp>

#include <unordered_map>

namespace ansipp {

std::unordered_map<std::string_view, input_type> escapes = {
    {"\x1b[A", input_type::UP},
    {"\x1b[B", input_type::DOWN},
    {"\x1b[C", input_type::RIGHT},
    {"\x1b[D", input_type::LEFT},
};

void parse_input_escapes(
    [[maybe_unused]] std::string_view& input, 
    [[maybe_unused]] std::vector<input_event>& events, 
    [[maybe_unused]] bool consume_all
) {}

}