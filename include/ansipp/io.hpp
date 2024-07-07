#pragma once

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
 * @return actual amount of bytes was written, or `0` in case of `EOF` (`errno == 0`) or any other error (`errno != 0`)
 */
std::size_t terminal_write(const void* buf, std::size_t sz);

/**
 * @brief simple non-bufferring function to write string to `stdout`.
 *
 * Useful for writing escape sequences. Note that string in this case considered as byte string.
 * 
 * @param str string to write
 * @return actual amount of bytes was written, or `0` in case of `EOF` (`errno == 0`) or any other error (`errno != 0`)
 * @ref terminal_write(const void*, std::size_t)
 */
std::size_t terminal_write(std::string_view str);

/**
 * @brief simple non-bufferring function to read raw bytes from `stdin`.
 *
 * Useful for reading keys (escape sequences) from `stdin`.
 * 
 * - `fread` - will wait until full buffer will be filled
 * - `std::cin.readsome()` - doesn't work by default in Linux (requires `std::cin.sync_with_stdio(false)`), 
 *      and never works on MacOS 
 * 
 * @param buf buffer to read
 * @param max_size maximum amount of bytes to read
 * @return actual amount of bytes was read, or `0` in case of `EOF` (`errno == 0`) or any other error (`errno != 0`)
 */
std::size_t terminal_read(void* buf, std::size_t max_size);

/**
 * @brief simple non-bufferring function to read raw bytes from `stdin` to `std::string`.
 * 
 * @param str destination string
 * @param max_size maximum amount of bytes to read
 * @return `true` if read was successful, `false` if `EOF` or error ocurred
 * @ref terminal_read(void*, std::size_t)
 */
bool terminal_read(std::string& str, std::size_t max_size = 20);


}