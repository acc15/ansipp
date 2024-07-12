#include <ansipp/stream.hpp>
#include <ansipp/io.hpp>

namespace ansipp {

int terminal_stringbuf::sync() {
    std::string_view v = view();
    if (v.empty()) return 0;
    if (std::streamsize sz = terminal_write(v); sz < 0 || static_cast<std::size_t>(sz) != v.size()) return -1;
    str(std::string());
    return 0;
}

terminal_stream::terminal_stream(): std::ostream(&buf) {}

}