#include <ansipp/io.hpp>

#ifdef _WIN32
#   include <windows.h>
#else
#   include <unistd.h>
#   include <poll.h>
#endif

namespace ansipp {

std::streamsize terminal_write(const void* buf, std::size_t sz) {
    if (sz == 0) {
        return 0; // fast return to avoid syscall
    }
#ifdef _WIN32
    DWORD result;
    return WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), buf, static_cast<DWORD>(sz), &result, nullptr) ? result : -1;
#else
    return write(STDOUT_FILENO, buf, sz);
#endif
}

std::streamsize terminal_read(void* buf, std::size_t sz) {
#ifdef _WIN32
    DWORD result;
    return ReadFile(GetStdHandle(STD_INPUT_HANDLE), buf, static_cast<DWORD>(sz), &result, nullptr) ? result : -1;
#else
    return read(STDIN_FILENO, buf, sz);
#endif
}

std::streamsize terminal_read(void* buf, std::size_t sz, int timeout) {
    const int v = timeout < 0 ? 1 : terminal_read_ready(timeout);
    return v > 0 ? terminal_read(buf, sz) : v;
}

std::streamsize terminal_write(std::string_view sw) {
    return terminal_write(sw.data(), sw.size());
}

int terminal_getch(int timeout) {
    char v[1];
    return terminal_read(v, 1, timeout) == 1 ? v[0] : -1;
}

int terminal_read_ready(int timeout) {
#ifdef _WIN32
    DWORD result = WaitForSingleObject(
        GetStdHandle(STD_INPUT_HANDLE), 
        timeout < 0 ? INFINITE : static_cast<DWORD>(timeout));
    switch (result) {
    case WAIT_OBJECT_0: return 1;
    case WAIT_FAILED: return -1;
    default: return 0;
    }
#else
    static pollfd stdin_pollfd = { .fd = STDIN_FILENO, .events = POLLIN, .revents = 0 };
    return poll(&stdin_pollfd, 1, timeout);
#endif
}

}