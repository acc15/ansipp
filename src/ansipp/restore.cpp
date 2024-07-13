#include <string>

#include <ansipp/restore.hpp>
#include <ansipp/attrs.hpp>
#include <ansipp/io.hpp>
#include <ansipp/cursor.hpp>
#include <ansipp/terminal.hpp>

#ifdef _WIN32
#   include <windows.h>
#else
#   include <unistd.h>
#endif

#include "restore.hpp"

namespace ansipp {

restore_data __ansipp_restore;

void restore_mode() {
#ifdef _WIN32 // windows
    __ansipp_restore.in_modes.restore([](DWORD in_modes) {
        SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), in_modes);
    });
    __ansipp_restore.out_modes.restore([](DWORD out_modes) {
        SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), out_modes);
    });
#else
    __ansipp_restore.lflag.restore([](tcflag_t lflag) {
        termios p;
        if (tcgetattr(STDOUT_FILENO, &p) == 0) {
            p.c_lflag = lflag;
            tcsetattr(STDOUT_FILENO, TCSAFLUSH, &p);
        }
    });
#endif
}

void restore() {
    __ansipp_restore.init_config.restore([](const config& cfg) {
        std::string esc;
        if (cfg.reset_attrs_on_restore) esc += attrs().str();
        if (cfg.use_alternate_screen_buffer) esc += disable_alternate_buffer();
        if (cfg.hide_cursor) esc += show_cursor();
        terminal_write(esc);
    });
    restore_mode();
}

}