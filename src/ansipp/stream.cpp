#include <ansipp/stream.hpp>
#include <ansipp/io.hpp>

namespace ansipp {

int terminal_stringbuf::sync() {
    std::string_view v = view();
    if (v.empty()) return 0;
    if (!terminal_write(v)) return -1;
    str(std::string());
    return 0;
}

terminal_stream::terminal_stream(): std::ostream(&buf) {}

}