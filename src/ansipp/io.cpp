#include <ansipp/io.hpp>

#ifdef _WIN32
#   include <windows.h>
#else
#   include <unistd.h>
#   include <poll.h>
#endif

namespace ansipp {

std::streamsize terminal_write(const void* buf, std::size_t sz) {
    if (sz == 0) return 0; // fast return to avoid syscall
#ifdef _WIN32
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    if (out == INVALID_HANDLE_VALUE) return -1;
    
    DWORD result;
    return WriteFile(out, buf, static_cast<DWORD>(sz), &result, nullptr) ? result : -1;
#else
    return write(STDOUT_FILENO, buf, sz);
#endif
}

std::streamsize terminal_read(void* buf, std::size_t sz) {
#ifdef _WIN32
    HANDLE in = GetStdHandle(STD_INPUT_HANDLE);
    if (in == INVALID_HANDLE_VALUE) return -1;
    
    DWORD result;
    return ReadFile(in, buf, static_cast<DWORD>(sz), &result, nullptr) ? result : -1;
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
    
    HANDLE in = GetStdHandle(STD_INPUT_HANDLE);
    if (in == INVALID_HANDLE_VALUE) return -1;

    DWORD result = WaitForSingleObject(in, timeout < 0 ? INFINITE : static_cast<DWORD>(timeout));
    if (result == WAIT_FAILED) {
        return -1;
    } else if (result != WAIT_OBJECT_0) {
        return 0;
    }
    
    // STD_INPUT_HANDLE will be signaled on ANY terminal event.
    // Even with DISABLED ENABLE_WINDOW_INPUT flag - SetConsoleMode(modes & (~ENABLE_WINDOW_INPUT))
    // ReadFile unable to read WINDOW_BUFFER_SIZE_EVENT 
    // and therefore it will block if all events of WINDOW_BUFFER_SIZE_EVENT type.
    // So the next is workaround: 
    // * check that input buffer has at least one KEY_EVENT or MOUSE_EVENT
    // * if not - consume all pending events and report as empty input

    const DWORD max_record_count = 32;
    INPUT_RECORD records[max_record_count];
    DWORD record_count;
    do {
        if (!PeekConsoleInput(in, records, max_record_count, &record_count)) return -1;
        if (record_count == 0) return 0;

        for (DWORD i = 0; i < record_count; ++i) {
            switch (records[i].EventType) {
                case KEY_EVENT: case MOUSE_EVENT: return 1;
                default: continue;
            }
        }

        // didn't find neither KEY_EVENT nor MOUSE_EVENT - consume all events
        if (!ReadConsoleInput(in, records, max_record_count, &record_count)) return -1;
    } while (record_count == max_record_count);

    // we consumed all ConsoleInput events and didn't find any KEY_EVENT or MOUSE_EVENT, returning 0 (no data to read)
    return 0;
#else
    pollfd stdin_pollfd = { .fd = STDIN_FILENO, .events = POLLIN, .revents = 0 };
    return poll(&stdin_pollfd, 1, timeout);
#endif
}

}