#include <string>
#include <ostream>
#include <system_error>

#include <ansipp/error.hpp>
#include <ansipp/io.hpp> 
#include <ansipp/attrs.hpp>

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
    return std::string("\33" "[")
        .append(1, static_cast<char>(mode))
        .append(1, static_cast<char>(target));
}

}