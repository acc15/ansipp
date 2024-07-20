#include <cstdlib>
#include <curses.h>
#include <iostream>
#include <locale>

int main() {
    std::locale::global(std::locale(""));

    initscr();
    if (!has_colors()) {
        endwin();
        std::cerr << "no color support!" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    addstr("󱫌󱫌󱫌!!! hi\n");
    getch();

    endwin();
    return EXIT_SUCCESS;
}