#include <iostream>
#include <iomanip>
#include <format>
#include <ansipp.hpp>

using namespace ansipp;

std::string encode_bytes(std::string_view str) {
    std::string result;
    for (auto it = str.begin(); it != str.end(); ++it) {
        if (it != str.begin()) result.append(1, ' ');
        result.append(std::format("0x{:02X}", *it));
    }
    return result;
}

std::string encode_string(std::string_view str) {
    std::string result;
    for (const char ch: str) {
        if (ch >= 0x20 && ch < 0x7f) {
            result.append(1, ch);
        } else {
            result.append(std::format("\\x{:02x}", ch));
        }
    }
    return result;
}

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
    const char* name;
    const char* esc_prefix;
    bool state = false;
};

mode_switch modes[] {
    mode_switch { .name = "X&Y",    .esc_prefix = "\33" "[?9" },
    mode_switch { .name = "Hilite", .esc_prefix = "\33" "[?1001" },
    mode_switch { .name = "Cell",   .esc_prefix = "\33" "[?1002" },
    mode_switch { .name = "All",    .esc_prefix = "\33" "[?1003" },
    mode_switch { .name = "Focus",  .esc_prefix = "\33" "[?1004" },
    mode_switch { .name = "UTF-8",  .esc_prefix = "\33" "[?1005" },
    mode_switch { .name = "SGR",    .esc_prefix = "\33" "[?1006" },
    mode_switch { .name = "Scroll", .esc_prefix = "\33" "[?1007" },
    mode_switch { .name = "urxvt",  .esc_prefix = "\33" "[?1015" },
};

void status_line(std::ostream& out) {
    out << store_cursor() << move_abs(1, 1) << attrs().bg(WHITE).fg(BLACK) << erase(LINE, TO_END);
    for (const mode_switch mode: modes) {
        out << ' ' << mode.name << ":" << mode.state;
    }
    out << attrs() << restore_cursor();
}

void parse_input(std::ostream& out, std::string_view str) {
    bool next_esc = false;
    while (!str.empty()) {
        char ch = str[0];
        str.remove_prefix(1);
        if (ch == '\33') {
            next_esc = true;
            continue;
        }
        if (next_esc && ch >= '1' && ch <= '1' + static_cast<char>(std::size(modes))) {
            mode_switch& mode = modes[ch - '1'];
            mode.state = !mode.state;
            out << mode.esc_prefix << (mode.state ? 'h' : 'l');
        }
        next_esc = false;
    }
}

int main() {

    if (std::error_code ec; init(ec, config { .disable_input_echo=true }), ec) {
        std::cerr << "can't init: " << ec.message() << std::endl;
        return EXIT_FAILURE;
    }

    terminal_stream out;
    
    char buf[512];

    status_line(out);
    out << std::flush;

    while (true) {
        std::string_view rd;
        if (!terminal_read(buf, rd)) { 
            std::cerr << "can't read stdin: " << last_error().message() << std::endl;
            return EXIT_FAILURE;
        }

        parse_input(out, rd);
        
        out << attrs().fg(GREEN) << "HEX: " << attrs() << encode_bytes(rd) 
            << attrs().fg(BLUE) << " CHARS: " << attrs() << encode_string(rd) 
            << '\n';
        status_line(out);
        out << std::flush;
    }
    return EXIT_SUCCESS;
}