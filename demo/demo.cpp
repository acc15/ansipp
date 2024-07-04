#include <iostream>
#include <ansipp.hpp>

using namespace ansipp;

void gradient(const rgb& a, const rgb& b, size_t width) {
    for (size_t i = 0; i < width; i++) {
        std::cout << attrs().bg(rgb::lerp(a, b, static_cast<float>(i) / (width - 1))) << " ";
    }
}

int main() {

    init();
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

    std::cout << "press " << attrs().on(BOLD).fg(GREEN) << "y" << attrs() << " to exit" << std::endl;
    std::cout << "type something" << std::endl;

    std::size_t pos = 0;
    char ch;

    while ((ch = std::cin.get()) != 'y') {
        std::cout << save_position() << move(UP) << move(TO_COLUMN, pos) << erase(LINE, ALL) << ch << restore_position() << std::flush;
    }
 
    restore();
    return 0;
}