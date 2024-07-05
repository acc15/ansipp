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
    #include <signal.h>
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

struct ansipp_restore {

#ifdef _WIN32 // windows
    std::optional<DWORD> modes;
#else // posix
    std::optional<tcflag_t> c_lflag;
    std::optional<struct sigaction> old_sigint; 
#endif

} __ansipp_restore;

const char __ansipp_reset[] = "\033[m\033[?25h";

#ifndef _WIN32
void sigint_reset(int code) {
    write(STDOUT_FILENO, __ansipp_reset, sizeof(__ansipp_reset));
    std::_Exit(0x80 + code);
}
#endif

bool init(const config& cfg) {
#ifdef _WIN32 // windows
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    
    DWORD dwModes;
    if (!GetConsoleMode(out, &dwModes)) {
        return false;
    }

    __ansipp_restore.modes = dwModes;
    if (cfg.disable_input_echo) {
        dwModes &= ~(
            ENABLE_ECHO_INPUT | 
            ENABLE_INSERT_MODE | 
            ENABLE_LINE_INPUT | 
            ENABLE_QUICK_EDIT_MODE
        );
        dwModes |= DISABLE_NEWLINE_AUTO_RETURN;
    }
    
    // forcibly enable virtual terminal processing
    dwModes |= (
        ENABLE_VIRTUAL_TERMINAL_PROCESSING | 
        ENABLE_VIRTUAL_TERMINAL_INPUT |
        ENABLE_PROCESSED_OUTPUT | 
        ENABLE_PROCESSED_INPUT
    );

    return SetConsoleMode(out, dwModes);
#else // posix
    if (cfg.disable_input_echo) {
        termios p;
        if (tcgetattr(STDOUT_FILENO, &p) != 0) {
            return false;
        }
        __ansipp_restore.c_lflag = p.c_lflag;
        p.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL); 
        if (tcsetattr(STDOUT_FILENO, TCSANOW, &p) != 0) {
            return false;
        }
    }
    if (cfg.enable_sigint_reset) {
        struct sigaction sa = { {&sigint_reset}, 0, 0 };
        struct sigaction osa;
        if (sigaction(SIGINT, &sa, &osa) == 0) {
            __ansipp_restore.old_sigint = osa;
        }
    }
    return true;
#endif
}

void restore() {
    std::cout << __ansipp_reset << std::flush;
    
#ifdef _WIN32 // windows
    if (__ansipp_restore.modes.has_value()) {
        SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), __ansipp_restore.modes.value());
        __ansipp_restore.modes.reset();
    }
#else
    if (__ansipp_restore.c_lflag.has_value()) {
        termios p;
        if (tcgetattr(STDOUT_FILENO, &p) == 0) {
            p.c_lflag = __ansipp_restore.c_lflag.value();
            tcsetattr(STDOUT_FILENO, TCSANOW, &p);
        }
        __ansipp_restore.c_lflag.reset();
    }
#endif
}

terminal_dimension get_terminal_dimension() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO ws;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &ws);
    return terminal_dimension {
        static_cast<unsigned short>(ws.srWindow.Bottom - ws.srWindow.Top + 1), 
        static_cast<unsigned short>(ws.srWindow.Right - ws.srWindow.Left + 1)
    };
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

std::size_t read_stdin(void* buf, std::size_t sz) {
#ifdef _WIN32
    DWORD dwRead;
    ReadConsole(GetStdHandle(STD_INPUT_HANDLE), buf, static_cast<DWORD>(sz), &dwRead, nullptr);
    return dwRead;
#else
    return read(STDIN_FILENO, buf, sz);
#endif
}

}