#include <iostream>
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

bool init(const config& cfg) {
#ifdef _WIN32 // windows
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    
    DWORD dwModes;
    if (!GetConsoleMode(out, &dwModes)) {
        return false;
    }

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
        ENABLE_PROCESSED_OUTPUT | 
        ENABLE_PROCESSED_INPUT | 
        ENABLE_VIRTUAL_TERMINAL_INPUT
    );

    return SetConsoleMode(out, dwModes);
#else // posix (linux, macos)
    if (cfg.no_echo) {
        termios p;
        if (tcgetattr(STDOUT_FILENO, &p) != 0) {
            return false;
        }
        p.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL); 
        if (tcsetattr(STDOUT_FILENO, TCSANOW, &p) != 0) {
            return false;
        }
    }
    return true;
#endif
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
    std::cout << request_cursor_pos() << std::flush;
    unsigned int row, col;
    if (scanf("\33[%u;%uR", &row, &col) != 2) {
        throw std::runtime_error("can't parse cursor position escape sequence");
    }
    return cursor_position { static_cast<unsigned short>(row), static_cast<unsigned short>(col) };
}




}