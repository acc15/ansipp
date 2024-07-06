#include <string>
#include <ostream>
#include <system_error>

#include <ansipp/error.hpp>
#include <ansipp/io.hpp> 

namespace ansipp {

/**
 * @brief checks whether stdin and stdout attached to terminal
 */
bool is_terminal();

struct config {
    
    /**
     * @brief disables terminal input echo
     */
    bool disable_input_echo = true;

    /**
     * @brief enables SIGINT signal handler which reset colors/styles and makes cursor visible (if it was hidden)
     */
    bool enable_signal_restore = true;

    /**
     * @brief enables exit handler which reset colors/styles and makes cursor visible (if it was hidden)
     */
    bool enable_exit_restore = true;

    /**
     * @brief changes windows console encoding to UTF-8
     */
    bool enable_utf8 = true;

};

enum init_status {
    OK,
    NOT_TERMINAL,
    ERROR_DISABLE_ECHO,
    ERROR_SIGINT
};

void init(std::error_code& ec, const config &cfg = {});
void restore();

struct terminal_dimension {
    unsigned short rows;
    unsigned short cols;
};
inline std::ostream& operator<<(std::ostream& o, const terminal_dimension& d) {
    return o << d.cols << "x" << d.rows;
}
terminal_dimension get_terminal_dimension();

struct cursor_position {
    unsigned short row;
    unsigned short col;
};
inline std::ostream& operator<<(std::ostream& o, const cursor_position& p) {
    return o << p.col << "," << p.row;
}
cursor_position get_cursor_position();

inline std::string show_cursor() { return "\33" "[?25h"; }
inline std::string hide_cursor() { return "\33" "[?25l"; }

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

inline std::string move(move_mode mode, unsigned int value = 1) {
    return std::string("\33" "[").append(std::to_string(value)).append(1, static_cast<char>(mode));
}

inline std::string move(unsigned short row, unsigned short col) {
    return std::string("\33" "[")
        .append(std::to_string(row)).append(1, ';')
        .append(std::to_string(col)).append(1, 'H');
}

inline std::string move(const cursor_position& p) { return move(p.row, p.col); }

inline std::string save_position() { return "\33" "7"; }
inline std::string restore_position() { return "\33" "8"; }
inline std::string request_position() { return "\33" "[6n"; }

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
    return std::string("\33" "[").append(1, static_cast<char>(mode)).append(1, static_cast<char>(target));
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

    static rgb lerp(const rgb& a, const rgb& b, float factor);

};

/**
 * @brief class for building style and color escape codes
 * 
 * Styles
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

    /**
     * @brief sets specified foreground/background color
     * @param bg `false` - foreground, `true` - background
     * @param v color to set
     * @param bright bright mode (odd terminal support)
     * @return self
     */
    inline attrs& c(bool bg, color v, bool bright) { return a(cb(bg, 30) + (bright ? 90 : 0) + v); }
    
    /**
     * @brief sets specified foreground/background RGB color
     * @param bg `false` - foreground, `true` - background
     * @param v color to set
     * @return self
     */
    inline attrs& c(bool bg, const rgb& v) { return a(cb(bg)).a(2).a(v.r).a(v.g).a(v.b); }
    
    /**
     * @brief sets specified foreground/background 8-bit color
     * @param bg `false` - foreground, `true` - background
     * @param v color to set
     * @return self
     */
    inline attrs& c(bool bg, unsigned char v) { return a(cb(bg)).a(5).a(v); }
    
    /**
     * @brief sets default foreground/background color
     * @param bg `false` - foreground, `true` - background
     * @return self
     */
    inline attrs& c(bool bg) { return a(cb(bg, 39)); }

    /**
     * @brief sets specified foreground color
     * @param v color to set
     * @param bright bright mode (odd terminal support)
     * @return self
     */
    inline attrs& fg(color v, bool bright = false) { return c(false, v, bright); }
    
    /**
     * @brief sets specified foreground RGB color
     * @param v color to set
     * @return self
     */
    inline attrs& fg(const rgb& v) { return c(false, v); }

    /**
     * @brief sets specified foreground 8-bit color
     * @param v color to set
     * @return self
     */
    inline attrs& fg(unsigned char v) { return c(false, v); }
    
    /**
     * @brief sets default foreground color
     * @return self
     */
    inline attrs& fg() { return c(false); }

    /**
     * @brief sets specified background color
     * @param v color to set
     * @param bright bright mode (odd terminal support)
     * @return self
     */
    inline attrs& bg(color v, bool bright = false) { return c(true, v, bright); }
    
    /**
     * @brief sets specified background RGB color
     * @param v color to set
     * @return self
     */
    inline attrs& bg(const rgb& v) { return c(true, v); }
    
    /**
     * @brief sets specified background 8-bit color
     * @param v color to set
     * @return self
     */
    inline attrs& bg(unsigned char v) { return c(true, v); }
    
    /**
     * @brief sets default background color
     * @return self
     */
    inline attrs& bg() { return c(true); }

    /**
     * @brief enables specified style
     * @param s style to enable
     * @return self
     */
    inline attrs& on(style s) { return a(static_cast<unsigned int>(s)); }
    
    /**
     * @brief disables specified style
     * @param s style to disable
     * @return self
     */
    inline attrs& off(style s) { return a(20 + static_cast<unsigned int>(s == style::BOLD ? style::DIM : s)); }

    /**
     * @brief disables all styles and colors
     * @return self
     */
    inline attrs& off() { return a(0); }

    /**
     * @brief builds ansi sequence
     * @return ansi sequence 
     */
    inline const std::string& str() { return value.append(1, 'm'); }
    
    /**
     * @brief builds ansi sequence
     * @return ansi sequence 
     */
    inline operator const std::string&() { return str(); }

};
inline std::ostream& operator<<(std::ostream& s, attrs&& a) { return s << a.str(); }
inline std::ostream& operator<<(std::ostream& s, attrs& a) { return s << a.str(); }

}