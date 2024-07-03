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

bool init(const config& cfg);
void restore();

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

constexpr std::string save_screen() {
    return "\033?47l";
}

constexpr std::string restore_screen() {
    return "\033?47h";
}

constexpr std::string enable_alternative_buffer() {
    return "\033[?1049h";
}

constexpr std::string disable_alternative_buffer() {
    return "\033[?1049l";
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
    return std::string("\33[").append(1, static_cast<char>(mode)).append(1, static_cast<char>(target));
}

enum class attr {
    RESET = 0,
    BOLD = 1,
    DIM = 2,
    ITALIC = 3,
    UNDERLINE = 4,
    BLINK = 5,
    INVERSE = 7,
    HIDDEN = 8,
    STRIKETHROUGH = 9,
};

/*constexpr std::string attrs() {

}*/


// ESC[0m		    reset all modes (styles and colors)
// ESC[1m	ESC[22m	set bold mode.
// ESC[2m	ESC[22m	set dim/faint mode.
// ESC[3m	ESC[23m	set italic mode.
// ESC[4m	ESC[24m	set underline mode.
// ESC[5m	ESC[25m	set blinking mode
// ESC[7m	ESC[27m	set inverse/reverse mode
// ESC[8m	ESC[28m	set hidden/invisible mode
// ESC[9m	ESC[29m	set strikethrough mode.

// Black	        30	40
// Red	            31	41
// Green	        32	42
// Yellow	        33	43
// Blue	            34	44
// Magenta	        35	45
// Cyan	            36	46
// White	        37	47
// Custom           38  48  5;n (8bit) 2;r;g;b (24bit)
// Default	        39	49

// Bright Black	    90	100
// Bright Red	    91	101
// Bright Green	    92	102
// Bright Yellow	93	103
// Bright Blue	    94	104
// Bright Magenta	95	105
// Bright Cyan	    96	106
// Bright White	    97	107

enum class color {
    BLACK,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
    WHITE
};

struct bright_color {
    color c;
};

struct color_rgb {
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

// attrs(attr::RESET, )

// fg()
// fg(bright(color::RED)) // bright colors
// fg(color::BLACK), 
// fg(244) - 8bit color
// fg(rgb { 155, 3, 3 }) - 24bit color
// fg(155, 3, 3) - 24bit color

}