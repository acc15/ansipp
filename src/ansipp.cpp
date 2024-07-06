#include <iostream>
#include <optional>
#include <cmath>
#include <cassert>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/ioctl.h>
    #include <termios.h>
    #include <unistd.h>
    #include <signal.h>
#endif

#include <ansipp.hpp>

#include "ansipp/ts_opt.hpp"

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
    ts_opt<DWORD> in_modes;
    ts_opt<DWORD> out_modes;
#else // posix
    ts_opt<tcflag_t> lflag;
#endif
} __ansipp_restore;

const std::string __ansipp_reset = attrs().str() + show_cursor();

void restore_mode() {
#ifdef _WIN32 // windows
    __ansipp_restore.in_modes.restore([](DWORD in_modes) {
        SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), in_modes);
    });
    __ansipp_restore.out_modes.restore([](DWORD out_modes) {
        SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), out_modes);
    });
#else
    __ansipp_restore.lflag.restore([](tcflag_t lflag) {
        termios p;
        if (tcgetattr(STDOUT_FILENO, &p) == 0) {
            p.c_lflag = lflag;
            tcsetattr(STDOUT_FILENO, TCSANOW, &p);
        }
    });
#endif
}

void restore() {
    terminal_write(__ansipp_reset.data(), __ansipp_reset.size());
    restore_mode();
}

void enable_utf8([[maybe_unused]] std::error_code& ec) {
#ifdef _WIN32
    UINT in_cp = GetConsoleCP(); 
    if (in_cp == 0 || (in_cp != CP_UTF8 && !SetConsoleCP(CP_UTF8))) { ec = last_error(); return; }
    
    UINT out_cp = GetConsoleOutputCP(); 
    if (out_cp == 0 || (out_cp != CP_UTF8 && !SetConsoleOutputCP(CP_UTF8))) { ec = last_error(); return; }
#endif
}

void signal_restore(int code) {
    restore();
    std::_Exit(0x80 + code);
}

#ifdef _WIN32
int signal_control_handler(DWORD ctrl_code) {
    if (ctrl_code == CTRL_C_EVENT) {
        signal_restore(2);
    }
    return false;
}
#endif

void enable_signal_restore(std::error_code& ec) {
#ifdef _WIN32 
    if (!SetConsoleCtrlHandler(&signal_control_handler, true)) { ec = last_error(); return; }
#else
    struct sigaction sa;
    sa.sa_handler = &signal_restore;
    if (sigaction(SIGINT, &sa, nullptr) == -1) { ec = last_error(); return; }
#endif
}

void configure_mode(std::error_code& ec, const config& cfg) {
#ifdef _WIN32 // windows

    HANDLE in = GetStdHandle(STD_INPUT_HANDLE);
    if (in == INVALID_HANDLE_VALUE) { ec = last_error(); return; }
    
    DWORD in_modes;
    if (!GetConsoleMode(in, &in_modes)) { ec = last_error(); return; }
    if (!__ansipp_restore.in_modes.store(in_modes)) { ec = ansipp_error::already_initialized; return; }
    if (cfg.disable_input_echo) {
        in_modes &= ~(
            ENABLE_ECHO_INPUT | 
            ENABLE_INSERT_MODE | 
            ENABLE_LINE_INPUT | 
            ENABLE_QUICK_EDIT_MODE |
            ENABLE_WINDOW_INPUT |
            ENABLE_MOUSE_INPUT
        );
    }
    in_modes |= (ENABLE_PROCESSED_INPUT | ENABLE_VIRTUAL_TERMINAL_INPUT);
    if (!SetConsoleMode(in, in_modes)) { ec = last_error(); return; }
    
    HANDLE out;
    if (out = GetStdHandle(STD_OUTPUT_HANDLE); out == INVALID_HANDLE_VALUE) { ec = last_error(); return; }
    
    DWORD out_modes; 
    if (!GetConsoleMode(out, &out_modes)) { ec = last_error(); return; }

    if (!__ansipp_restore.out_modes.store(out_modes)) { ec = ansipp_error::already_initialized; return; }
    out_modes |= (ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    if (!SetConsoleMode(out, out_modes)) { ec = last_error(); return; }

#else // posix
    if (cfg.disable_input_echo) {
        termios p; 
        if (tcgetattr(STDOUT_FILENO, &p) == -1) { ec = last_error(); return; }
        if (!__ansipp_restore.lflag.store(p.c_lflag)) { ec = ansipp_error::already_initialized; return; }
        p.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL); 
        if (tcsetattr(STDOUT_FILENO, TCSANOW, &p) == -1) { ec = last_error(); return; }
    }
#endif
}

void init(std::error_code& ec, const config& cfg) {
    if (cfg.enable_exit_restore && std::atexit(&restore) != 0) { ec = ansipp_error::at_exit_failure; return; }
    if (cfg.enable_signal_restore && (enable_signal_restore(ec), ec)) { return; }
    if (cfg.enable_utf8 && (enable_utf8(ec), ec)) { return; }
    if (configure_mode(ec, cfg), ec) { return; }
}

terminal_dimension get_terminal_dimension() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO ws;
    return GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &ws) 
        ? terminal_dimension {
            static_cast<unsigned short>(ws.srWindow.Bottom - ws.srWindow.Top + 1),
            static_cast<unsigned short>(ws.srWindow.Right - ws.srWindow.Left + 1)
        } : terminal_dimension {};
#else
    winsize ws;
    return ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) != -1 
        ? terminal_dimension { ws.ws_row, ws.ws_col }
        : terminal_dimension {};
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

}