#include <string>
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

enum move_mode: char {
    UP = 'A', 
    DOWN = 'B', 
    RIGHT = 'C', 
    LEFT = 'D', 
    NEXT_LINE = 'E', 
    PREV_LINE = 'F', 
    TO_COLUMN = 'G',
    SCROLL_UP = 'S',
    SCROLL_DOWN = 'T'
};

inline std::string move(move_mode mode, unsigned int amount) {
    return std::string("\33[").append(std::to_string(amount)).append(1, static_cast<char>(mode));
}

inline std::string move(const cursor_position& p) {
    return std::string("\33[")
        .append(std::to_string(p.row)).append(1, ';')
        .append(std::to_string(p.col)).append(1, 'H');
}

constexpr std::string save_position() {
    return "\0337";
}

constexpr std::string restore_position() {
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

constexpr std::string request_position() {
    return "\33[6n";
}

enum erase_target: char {
    LINE = 'J',
    SCREEN = 'K'
};

enum erase_mode: char {
    TO_END = '0',
    TO_BEGIN = '1',
    ALL = '2'
};

constexpr std::string erase(erase_target target, erase_mode mode) {
    return std::string("\33[").append(1, static_cast<char>(mode)).append(1, static_cast<char>(target));
}

enum style {
    BOLD = 1,
    DIM,
    ITALIC,
    UNDERLINE,
    BLINK,
    INVERSE,
    HIDDEN,
    STRIKETHROUGH
};

enum color {
    BLACK,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
    WHITE
};

struct rgb {
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

/**
 * @brief class for building style and color escape codes
 * 
 * ## Styles
 *
 * | Enable | Disable | Description                         |
 * |--------|---------|-------------------------------------|
 * | ESC[0m |         | reset all modes (styles and colors) |
 * | ESC[1m | ESC[22m | set bold mode.                      |
 * | ESC[2m | ESC[22m | set dim/faint mode.                 |
 * | ESC[3m | ESC[23m | set italic mode.                    |
 * | ESC[4m | ESC[24m | set underline mode.                 |
 * | ESC[5m | ESC[25m | set blinking mode                   |
 * | ESC[7m | ESC[27m | set inverse/reverse mode            |
 * | ESC[8m | ESC[28m | set hidden/invisible mode           |
 * | ESC[9m | ESC[29m | set strikethrough mode.             |
 * 
 * ## Colors
 *
 * | Color   | FG | BG |
 * |---------|----|----|
 * | Black   | 30 | 40 |
 * | Red     | 31 | 41 |
 * | Green   | 32 | 42 |
 * | Yellow  | 33 | 43 |
 * | Blue    | 34 | 44 |
 * | Magenta | 35 | 45 |
 * | Cyan    | 36 | 46 |
 * | White   | 37 | 47 |
 * | Custom  | 38 | 48 | 5;n (8bit) 2;r;g;b (24bit)
 * | Default | 39 | 49 |
 * 
 * ## Bright colors
 *
 * | Color   | FG | BG  |
 * |---------|----|-----|
 * | Black   | 90 | 100 |
 * | Red     | 91 | 101 |
 * | Green   | 92 | 102 |
 * | Yellow  | 93 | 103 |
 * | Blue    | 94 | 104 |
 * | Magenta | 95 | 105 |
 * | Cyan    | 96 | 106 |
 * | White   | 97 | 107 |
 */
class attrs {

    std::string value = "\033[";

    attrs& a(unsigned int param);
    inline unsigned int cb(bool bg, unsigned int base = 38) { return base + (bg ? 10 : 0); }

public:
    attrs& c(bool bg, color v, bool bright = false);
    attrs& c(bool bg, const rgb& v);
    attrs& c(bool bg, unsigned char v);
    attrs& c(bool bg);

    inline attrs& fg(color v, bool bright = false) { return c(false, v, bright); }
    inline attrs& fg(const rgb& v) { return c(false, v); }
    inline attrs& fg(unsigned char v) { return c(false, v); }
    inline attrs& fg() { return c(false); }

    inline attrs& bg(color v, bool bright = false) { return c(true, v, bright); }
    inline attrs& bg(const rgb& v) { return c(true, v); }
    inline attrs& bg(unsigned char v) { return c(true, v); }
    inline attrs& bg() { return c(true); }

    inline attrs& on(style s) { return a(static_cast<unsigned int>(s)); }
    inline attrs& off(style s) { return a(20 + static_cast<unsigned int>(s == style::BOLD ? style::DIM : s)); }
    inline attrs& off() { return a(0); }

    inline const std::string& str() { return value.append(1, 'm'); }
    inline operator const std::string&() { return str(); }

};
std::ostream& operator<<(std::ostream& s, attrs& a);

}