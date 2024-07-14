#include <ansipp.hpp>
#include <iostream>

using namespace ansipp;

int main() {
    if (std::error_code ec; init(ec, {}), ec) {
        std::cerr << "can't init: " << ec.message() << std::endl;
        return EXIT_FAILURE;
    }

    terminal_stream out;
    // this won't work with scrolling...
    out << store_cursor() << "hello\nworld\n" << restore_cursor() << std::flush;
    terminal_getch();
    return 0;
}