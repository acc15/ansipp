#include <iostream>
#include <optional>
#include <cmath>
#include <ansipp.hpp>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/ioctl.h>
    #include <termios.h>
    #include <unistd.h>
#endif

namespace ansipp {

bool is_terminal() {
#ifdef _WIN32
    return GetFileType(GetStdHandle(STD_INPUT_HANDLE)) == FILE_TYPE_CHAR && 
        GetFileType(GetStdHandle(STD_OUTPUT_HANDLE)) == FILE_TYPE_CHAR;
#else
    return isatty(STDIN_FILENO) && isatty(STDOUT_FILENO);
#endif
}

static std::optional<
#ifdef _WIN32 // windows
    DWORD
#else // posix
    tcflag_t
#endif
> __ansipp_restore;

bool init(const config& cfg) {
#ifdef _WIN32 // windows
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    
    DWORD dwModes;
    if (!GetConsoleMode(out, &dwModes)) {
        return false;
    }

    __ansipp_restore = dwModes;
    if (cfg.no_echo) {
        dwModes &= ~(
            ENABLE_ECHO_INPUT | 
            ENABLE_INSERT_MODE | 
            ENABLE_LINE_INPUT | 
            ENABLE_QUICK_EDIT_MODE
        );
    }
    
    dwModes |= (
        ENABLE_VIRTUAL_TERMINAL_PROCESSING | 
        ENABLE_VIRTUAL_TERMINAL_INPUT |
        ENABLE_PROCESSED_OUTPUT | 
        ENABLE_PROCESSED_INPUT
    );

    return SetConsoleMode(out, dwModes);
#else // posix
    if (cfg.no_echo) {
        termios p;
        if (tcgetattr(STDOUT_FILENO, &p) != 0) {
            return false;
        }
        __ansipp_restore = p.c_lflag;
        p.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL); 
        if (tcsetattr(STDOUT_FILENO, TCSANOW, &p) != 0) {
            return false;
        }
    }
    return true;
#endif
}

void restore() {
    std::cout << attrs() << show_cursor() << std::flush;
    if (!__ansipp_restore.has_value()) {
        return;
    }
#ifdef _WIN32 // windows
    SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), __ansipp_restore.value());
#else
    termios p;
    if (tcgetattr(STDOUT_FILENO, &p) == 0) {
        p.c_lflag = __ansipp_restore.value();
        tcsetattr(STDOUT_FILENO, TCSANOW, &p);
    }
#endif
    __ansipp_restore.reset();
}

std::ostream& operator<<(std::ostream& o, const terminal_dimension& d) {
    return o << d.cols << "x" << d.rows;
}

std::ostream& operator<<(std::ostream& o, const cursor_position& p) {
    return o << p.col << "," << p.row;
}

terminal_dimension get_terminal_dimension() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO ws;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &ws);
    return terminal_dimension { ws.srWindow.Bottom - ws.srWindow.Top + 1, ws.srWindow.Right - ws.srWindow.Left + 1 };
#else
    winsize ws;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    return terminal_dimension { ws.ws_row, ws.ws_col };
#endif
}

cursor_position get_cursor_position() {
    std::cout << request_position() << std::flush;

    cursor_position p;
    // (ignore '\033' '[')(row)(ignore ';')(col)(ignore 'R')
    if (!((std::cin.ignore(2) >> p.row).ignore() >> p.col).ignore()) {
        throw std::runtime_error("can't parse cursor position escape sequence");
    }
    return p;
}

std::string move(move_mode mode, unsigned int value) {
    return std::string("\33[").append(std::to_string(value)).append(1, static_cast<char>(mode));
}

std::string move(unsigned short row, unsigned short col) {
    return std::string("\33[")
        .append(std::to_string(row)).append(1, ';')
        .append(std::to_string(col)).append(1, 'H');
}

rgb rgb::lerp(const rgb& a, const rgb& b, float factor) {
    return rgb { 
        static_cast<unsigned char>(std::lerp(a.r, b.r, factor)),
        static_cast<unsigned char>(std::lerp(a.g, b.g, factor)),
        static_cast<unsigned char>(std::lerp(a.b, b.b, factor))
    };
}

attrs& attrs::a(unsigned int param) {
    if (value.size() > 2) {
        value.append(1, ';');
    }
    value.append(std::to_string(param));
    return *this;
}

attrs& attrs::c(bool bg, color v, bool bright) { return a(cb(bg, 30) + (bright ? 90 : 0) + v); }
attrs& attrs::c(bool bg, const rgb& v) { return a(cb(bg)).a(2).a(v.r).a(v.g).a(v.b); }
attrs& attrs::c(bool bg, unsigned char v) { return a(cb(bg)).a(5).a(v); }
attrs& attrs::c(bool bg) { return a(cb(bg, 39)); }

}