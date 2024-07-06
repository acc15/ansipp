#include <ansipp/io.hpp>

#ifdef _WIN32
#   include <windows.h>
#else
#   include <unistd.h>
#endif

namespace ansipp {

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