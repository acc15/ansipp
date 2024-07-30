#include <ansipp.hpp>

using namespace ansipp;

void gradient(charbuf& out, const rgb& a, const rgb& b, size_t width) {
    for (size_t i = 0; i < width; i++) {
        out << attrs().bg(rgb::lerp(a, b, static_cast<float>(i) / (width - 1))) << " ";
    }
}

int main() {

    init_or_exit({ .hide_cursor = true });

    charbuf out(4096);
    out << attrs().on(UNDERLINE).fg(WHITE) << "hello" << attrs() << '\n'
        << attrs().fg(RED) << "i'm red" << attrs() << '\n'
        << attrs().fg(WHITE).bg(GREEN) << "i'm white on green bg" << attrs() << '\n';
    terminal_write(out.flush());

    vec dw = get_terminal_size();
    out << "terminal dimension = " << dw << '\n'
        << "next line must draw gradient using rgb escape codes: \n";

    gradient(out, { 255, 0, 0 }, { 0, 255, 0 }, 40);
    gradient(out, { 0, 255, 0 }, { 0, 0, 255 }, 40);
    out << attrs() << '\n'
        << "press " << attrs().on(BOLD).on(BLINK).fg(GREEN) << 'q' << attrs() << " to exit\n"
        << attrs().on(INVERSE) << "type something" << attrs() << '\n';
    terminal_write(out.flush());

    std::string seq_buf(20, '\0');
    std::string_view rd;
    do {
        if (!terminal_read(seq_buf, rd)) {
            stderr_write((out << "can't read stdin: " << last_error().message() << '\n').flush());
            return EXIT_FAILURE;
        }
        out << store_cursor
            << move(CURSOR_UP) << move(CURSOR_TO_COLUMN, 0) 
            << erase(LINE, ALL) << rd.size() << " chars received:";
        for (const char& c: rd) {
            out << " 0x" << integral_format(c, 16, true, 2);
        }
        out << restore_cursor;
        terminal_write(out.flush());
    } while (rd != "q");
    return 0;
}
