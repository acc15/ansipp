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
#include "ts_opt.hpp"

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
    tc_opt<DWORD> in_modes;
    tc_opt<DWORD> out_modes;
    tc_opt<UINT> in_cp = 0;
    tc_opt<UINT> out_cp = 0;
#else // posix
    ts_opt<tcflag_t> lflag;
#endif
} __ansipp_restore;

const std::string __ansipp_reset = attrs().str() + show_cursor();

void restore_utf8() {
#ifdef _WIN32 // windows
    __ansipp_restore.in_cp.restore([](UINT in_cp) { SetConsoleCP(in_cp); });
    __ansipp_restore.out_cp.restore([](UINT out_cp) { SetConsoleOutputCP(out_cp); });
#endif
}

void restore_mode() {
#ifdef _WIN32 // windows
    __ansipp_restore.in_modes.restore([](DWORD in_modes) {
        SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), in_modes);
    });
    __ansipp_restore.out_modes.restore([](DWORD out_modes) {
        SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), __ansipp_restore.out_modes.value());
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
    restore_utf8();
}

void sigint_restore(int code) {
    restore();
    std::_Exit(0x80 + code);
}

#ifdef _WIN32
int sigint_control_handler(DWORD ctrl_code) {
    if (ctrl_code == CTRL_C_EVENT) {
        sigint_restore(2);
    }
    return false;
}
#endif

std::string format_last_error() {
#ifdef _WIN32
    DWORD code = GetLastError();
    
    LPSTR temp_buf = nullptr;

    //Ask Win32 to give us the string version of that message ID.
    //The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
    DWORD sz = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, code, 
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
        reinterpret_cast<LPSTR>(&temp_buf), 
        0, 
        NULL);
    
    //Copy the error message into a std::string.
    std::string message(temp_buf, sz);
    
    //Free the Win32's string's buffer.
    LocalFree(temp_buf);

    return message;
#else
    return std::string(strerror(errno));
#endif
}

bool enable_utf8() {
#ifdef _WIN32

    UINT in_cp = GetConsoleCP();
    if (in_cp == 0) {
        return false;
    }
    if (in_cp != CP_UTF8) {
        __ansipp_restore.in_cp.store(in_cp);
        if (!SetConsoleCP(CP_UTF8)) {
            return false;
        }
    }

    UINT out_cp = GetConsoleOutputCP();
    if (out_cp == 0) {
        return false;
    }
    if (out_cp != CP_UTF8) {
        __ansipp_restore.out_cp.store(out_cp);
        if (!SetConsoleOutputCP(CP_UTF8)) {
            return false;
        }
    }

#endif
    return true;
}

bool enable_sigint_restore() {
#ifdef _WIN32 
    return SetConsoleCtrlHandler(&sigint_control_handler, true);
#else
    struct sigaction sa;
    sa.sa_handler = &sigint_restore;
    return sigaction(SIGINT, &sa, nullptr) == 0;
#endif
}

bool configure_mode(const config& cfg) {
#ifdef _WIN32 // windows

    HANDLE in = GetStdHandle(STD_INPUT_HANDLE);
    
    DWORD in_modes;
    if (!GetConsoleMode(in, &in_modes)) {
        return false;
    }

    __ansipp_restore.in_modes.store(in_modes);
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
    if (!SetConsoleMode(in, in_modes)) {
        return false;
    }

    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    
    DWORD out_modes;
    if (!GetConsoleMode(out, &out_modes)) {
        return false;
    }

    __ansipp_restore.out_modes.store(out_modes);
    out_modes |= (ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    if (!SetConsoleMode(out, out_modes)) {
        return false;
    }

#else // posix
    if (cfg.disable_input_echo) {
        termios p;
        if (tcgetattr(STDOUT_FILENO, &p) != 0) {
            return false;
        }
        __ansipp_restore.lflag.store(p.c_lflag);
        p.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL); 
        if (tcsetattr(STDOUT_FILENO, TCSANOW, &p) != 0) {
            return false;
        }
    }
#endif
    return true;
}

bool init(const config& cfg) {
    if (cfg.enable_utf8 && !enable_utf8()) { return false; }
    if (!configure_mode(cfg)) { return false; }
    if (cfg.enable_exit_restore && std::atexit(&restore) != 0) { return false; }
    if (cfg.enable_sigint_restore && !enable_sigint_restore()) { return false; }
    return true;
}

terminal_dimension get_terminal_dimension() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO ws;
    if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &ws)) {
        return terminal_dimension {};
    }
    return terminal_dimension {
        static_cast<unsigned short>(ws.srWindow.Bottom - ws.srWindow.Top + 1), 
        static_cast<unsigned short>(ws.srWindow.Right - ws.srWindow.Left + 1)
    };
#else
    winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) {
        return terminal_dimension {};
    }
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

std::size_t terminal_write(const void* buf, std::size_t sz) {
#ifdef _WIN32
    DWORD result;
    return WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), buf, static_cast<DWORD>(sz), &result, nullptr) ? result : 0;
#else
    ssize_t result = write(STDOUT_FILENO, buf, sz);
    return result < 0 ? 0 : result;
#endif
}

std::size_t terminal_read(void* buf, std::size_t sz) {
#ifdef _WIN32
    DWORD result;
    return ReadFile(GetStdHandle(STD_INPUT_HANDLE), buf, static_cast<DWORD>(sz), &result, nullptr) ? result : 0;
#else
    ssize_t result = read(STDIN_FILENO, buf, sz);
    return result < 0 ? 0 : result;
#endif
}

}