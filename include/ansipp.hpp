#include <string>
#include <format>
#include <ostream>

namespace ansipp {

/**
 * @brief checks whether stdin and stdout attached to terminal
 */
bool is_terminal();

struct config {
    /**
     * @brief enables/disables terminal input echo
     */
    bool no_echo = true;
};

struct restore {

};

bool init(const config& cfg);

void restore(config& cfg);



struct terminal_dimension {
    unsigned short rows;
    unsigned short cols;
};

std::ostream& operator<<(std::ostream& o, const terminal_dimension& d);

terminal_dimension get_terminal_dimension();

struct cursor_position {
    unsigned short row;
    unsigned short col;
};

std::ostream& operator<<(std::ostream& o, const cursor_position& p);

cursor_position get_cursor_position();

constexpr std::string show_cursor() {
    return "\33[?25h";
}

constexpr std::string hide_cursor() {
    return "\33[?25l";
}

enum class move_mode: char {
    UP = 'A', 
    DOWN = 'B', 
    RIGHT = 'C', 
    LEFT = 'D', 
    NEXT_LINE = 'E', 
    PREV_LINE = 'F', 
    HORIZONTAL_ABSOLUTE = 'G',
    SCROLL_UP = 'S',
    SCROLL_DOWN = 'T'
};

constexpr std::string move(move_mode mode, unsigned int amount) {
    return std::format("\33[{}{}", amount, static_cast<char>(mode));
}

constexpr std::string move(const cursor_position& p) {
    return std::format("\33[{};{}H", p.row, p.col);
}

constexpr std::string save_cursor_position() {
    return "\0337";
}

constexpr std::string restore_cursor_position() {
    return "\0338";
}

constexpr std::string reset_attrs() {
    return "\33[0m";
}

constexpr std::string request_cursor_pos() {
    return "\33[6n";
}

enum class erase_target: char {
    LINE = 'J',
    SCREEN = 'K'
};

enum class erase_mode: char {
    TO_END = '0',
    TO_BEGIN = '1',
    ALL = '2'
};

constexpr std::string erase(erase_target target, erase_mode mode) {
    return std::format("\33[{}{}", static_cast<char>(mode), static_cast<char>(target));
}


}