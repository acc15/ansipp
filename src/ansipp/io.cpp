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

std::size_t terminal_write(std::string_view sw) {
    return terminal_write(sw.data(), sw.size());
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

bool terminal_read(std::string& str, std::size_t max_size) {
    str.resize(max_size);
    str.resize(terminal_read(str.data(), str.size()));
    return !str.empty();
}

int terminal_getch() {
    char v[1];
    return terminal_read(v, 1) == 1 ? v[0] : -1;
}

}