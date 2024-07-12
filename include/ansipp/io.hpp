#pragma once

#include <ios>
#include <string>
#include <string_view>
#include <cstddef>

namespace ansipp {

/**
 * @brief simple non-bufferring function to write raw bytes to `stdout`.
 *
 * Useful for writing escape sequences.
 * It's safe to use this function in signal handler.
 * 
 * @param buf buffer to write
 * @param sz amount of bytes to write
 * @return actual amount of bytes was written, or `-1` in case of error
 */
std::streamsize terminal_write(const void* buf, std::size_t sz);
std::streamsize terminal_write(std::string_view str);

/**
 * @brief simple non-bufferring function to read raw bytes from `stdin`.
 *
 * @param buf buffer to read
 * @param sz maximum amount of bytes to read
 * @return actual amount of bytes was read, or `-1` in case of error
 */
std::streamsize terminal_read(void* buf, std::size_t sz);
std::streamsize terminal_read(void* buf, std::size_t sz, int timeout);

template <typename T>
bool terminal_read(T& buf, std::string_view& rd, int timeout = -1) {
    char* data = std::data(buf);
    auto res = terminal_read(data, std::size(buf), timeout); 
    if (res < 0) return false;
    rd = std::string_view(data, res);
    return true;
}

/**
 * @brief reads single char
 * @return read single char or `-1` if error ocurred
 */
int terminal_getch(); 

/**
 * @brief checks that terminal has any byte to read
 * @param timeout timeout in milliseconds, `0` return immediately, negative values - infinite timeout
 * @return 
 *   `1` - terminal has byte (or bytes) to read
 *   `0` - nothing to read (i.e. `terminal_read(void*, std::size_t)` would block)
 *   `-1` - in case of error
 */
int terminal_read_ready(int timeout = 0);

}