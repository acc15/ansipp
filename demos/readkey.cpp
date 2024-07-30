#include <iostream>
#include <iomanip>
#include <thread>
#include <ansipp.hpp>

using namespace ansipp;
/*
Ps = 9  -> Send Mouse X & Y on button press.  See the section Mouse Tracking.  This is the X10 xterm mouse protocol.
Ps = 1 0 0 1  -> Use Hilite Mouse Tracking.
Ps = 1 0 0 2  -> Use Cell Motion Mouse Tracking.
Ps = 1 0 0 3  -> Use All Motion Mouse Tracking.
Ps = 1 0 0 4  -> Send FocusIn/FocusOut events.
Ps = 1 0 0 5  -> Enable UTF-8 Mouse Mode.
Ps = 1 0 0 6  -> Enable SGR Mouse Mode.
Ps = 1 0 0 7  -> Enable Alternate Scroll Mode.
Ps = 1 0 1 0  -> Scroll to bottom on tty output (rxvt).
Ps = 1 0 1 1  -> Scroll to bottom on key press (rxvt).
Ps = 1 0 1 5  -> Enable urxvt Mouse Mode.
*/

struct mode_switch {
    const std::string name;
    const decset_mode& decset;
    const bool initial_value = false;
    bool value = initial_value;
};

mode_switch modes[] {
    mode_switch { .name = "X&Y",            .decset = mouse_click },
    mode_switch { .name = "Cell",           .decset = mouse_cell },
    mode_switch { .name = "All",            .decset = mouse_all },
    mode_switch { .name = "Focus",          .decset = focus_reporting },
    mode_switch { .name = "UTF-8",          .decset = mouse_utf8 },
    mode_switch { .name = "SGR",            .decset = mouse_sgr },
    mode_switch { .name = "Show Cursor",    .decset = cursor_visibility, .initial_value = true },
};

void mode_line(charbuf& out) {
    out << store_cursor << move_abs(1, 1) << attrs().bg(WHITE).fg(BLACK) << "<alt>";
    for (std::size_t i = 0; i < std::size(modes); i++) {
        const mode_switch& m = modes[i];
        out << ' ' << attrs().fg(RED) << static_cast<char>('a' + i) << attrs().fg(BLACK)
            << ':' << m.name 
            << '=' << attrs().on(BOLD) << m.value << attrs().off(BOLD);
    }
    out << erase(LINE, TO_END) << attrs() << restore_cursor;
}

void parse_input(charbuf& out, std::string_view str) {
    bool next_esc = false;
    while (!str.empty()) {
        char ch = str[0];
        str.remove_prefix(1);
        if (ch == esc[0]) {
            next_esc = true;
            continue;
        }
        if (next_esc && ch >= 'a' && ch <= 'a' + static_cast<char>(std::size(modes) - 1)) {
            mode_switch& mode = modes[ch - 'a'];
            mode.value = !mode.value;
            out << (mode.value ? mode.decset.on() : mode.decset.off());
        }
        next_esc = false;
    }
}

int main() {

    charbuf restore_esc;
    for (const mode_switch& m: modes) {
        restore_esc << (m.initial_value ? m.decset.on() : m.decset.off());
    }

    init_or_exit(config { .restore_esc = restore_esc.view() });

    charbuf out;
    
    mode_line(out);
    out << charbuf::to_stdout;

    char input_buf[512];
    while (true) {
        std::string_view rd;
        if (!stdin_read(input_buf, rd)) { 
            std::cerr << "can't read stdin: " << last_error().message() << std::endl;
            return EXIT_FAILURE;
        }

        parse_input(out, rd);
        if (!rd.empty()) {
            out << attrs().fg(GREEN) << "HEX:" << attrs();
            for (char ch: rd) {
                out << ' ' << integral_format<unsigned char>(ch, 16, true, 2);
            }
            out << attrs().fg(BLUE) << " CHARS: " << attrs();
            for (const char ch: rd) {
                if (ch >= 0x20 && ch < 0x7f) {
                    out << ch;
                } else {
                    out << attrs().fg(CYAN) << "0x" << integral_format<unsigned char>(ch, 16, false, 2) << attrs();
                }
            }
        } else {
            out << attrs().fg(color::YELLOW) << "<EMPTY>" << attrs();
        }
        out << '\n';
#ifdef _WIN32
        out << charbuf::to_stdout;
        // Allow Windows Terminal to update current line... otherwise mode_line sometimes won't draw 
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
#endif
        mode_line(out);
        out << charbuf::to_stdout;
    }
    return EXIT_SUCCESS;
}