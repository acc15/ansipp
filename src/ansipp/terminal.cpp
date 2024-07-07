#include <ansipp/terminal.hpp>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/ioctl.h>
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

}