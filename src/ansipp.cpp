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
    std::optional<DWORD> in_modes;
    std::optional<DWORD> out_modes;
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

bool init(const config& cfg) {
#ifdef _WIN32 // windows
    HANDLE in = GetStdHandle(STD_INPUT_HANDLE);
    
    DWORD in_modes;
    if (!GetConsoleMode(in, &in_modes)) {
        return false;
    }

    __ansipp_restore.in_modes = in_modes;
    if (cfg.disable_input_echo) {
        in_modes &= ~(
            ENABLE_ECHO_INPUT | 
            ENABLE_INSERT_MODE | 
            ENABLE_LINE_INPUT | 
            ENABLE_QUICK_EDIT_MODE |
            ENABLE_WINDOW_INPUT 
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

    __ansipp_restore.out_modes = out_modes;
    
    // forcibly enable virtual terminal processing
    out_modes |= (ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

    std::cout << "SetConsoleMode: " << std::hex << "0x" << out_modes << std::endl;
    if (!SetConsoleMode(out, out_modes)) {
        std::cout << "SetConsoleMode fail" << std::endl;
        return false;
    }
    std::cout << "SetConsoleMode OK" << std::endl;
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
#endif
    return true;
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