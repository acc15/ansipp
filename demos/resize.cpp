#include <iostream>
#include <iomanip>
#include <thread>
#include <ansipp.hpp>

using namespace ansipp;


int main() {

    if (std::error_code ec; init(ec), ec) {
        std::cerr << "can't init: " << ec.message() << std::endl;
        return EXIT_FAILURE;
    }

    terminal_stream o;

    // o << disable_line_wrap() << std::flush;
    // int ch;
    while (true) {
        // if (ch == 'w') o << disable_line_wrap();
        //else if (ch == 'e') o << enable_line_wrap();
        cursor_position p = get_cursor_position();
        
        o << erase(SCREEN, TO_END) << attrs().bg(color::BLUE).fg(color::WHITE) << std::string(200, 'a') << attrs() << '\n';
        // o << std::string(200, 'b') << '\n';
        o << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        o << move(p) << std::flush;
    }

    return 0;
}
