#include <iostream>
#include <iomanip>
#include <ansipp.hpp>

using namespace ansipp;

void gradient(const rgb& a, const rgb& b, size_t width) {
    for (size_t i = 0; i < width; i++) {
        std::cout << attrs().bg(rgb::lerp(a, b, static_cast<float>(i) / (width - 1))) << " ";
    }
}

int main() {

    if (std::error_code ec; init(ec), ec) {
        std::cerr << "can't init: " << ec.message() << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << hide_cursor();
    std::cout << attrs().on(UNDERLINE).fg(WHITE) << "hello" << attrs() << std::endl;
    std::cout << attrs().fg(RED) << "i'm red" << attrs() << std::endl;
    std::cout << attrs().fg(WHITE).bg(GREEN) << "i'm white on green bg" << attrs() << std::endl;

    terminal_dimension dw = get_terminal_dimension();
    std::cout << "terminal dimension = " << dw << std::endl;

    std::cout << "next line must draw gradient using rgb escape codes: " << std::endl;

    gradient({ 255, 0, 0 }, { 0, 255, 0 }, 40);
    gradient({ 0, 255, 0 }, { 0, 0, 255 }, 40);
    std::cout << attrs() << std::endl;

    std::cout << "press " << attrs().on(BOLD).fg(GREEN) << "q" << attrs() << " to exit" << std::endl;
    std::cout << "type something" << std::endl;

    std::string seq_buf;
    while (terminal_read(seq_buf) && seq_buf != "q") {
        std::cout << save_cursor() 
            << move(CURSOR_UP) << move(CURSOR_TO_COLUMN, 0) << erase(LINE, ALL) << std::dec << seq_buf.size() << " chars received:";
        for (const char& c: seq_buf) {
            std::cout << " 0x" 
                << std::hex << std::setw(2) << std::setfill('0') << std::uppercase 
                << static_cast<unsigned int>(static_cast<unsigned char>(c));
        }
        std::cout << restore_cursor() << std::flush;
    }
    return 0;
}
