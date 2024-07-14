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

vec get_terminal_dimension() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO ws;
    return GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &ws) 
        ? vec {
            static_cast<int>(ws.srWindow.Right - ws.srWindow.Left + 1),
            static_cast<int>(ws.srWindow.Bottom - ws.srWindow.Top + 1)
        } : vec {};
#else
    winsize ws;
    return ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) != -1 
        ? vec { ws.ws_col, ws.ws_row }
        : vec {};
#endif
}

}