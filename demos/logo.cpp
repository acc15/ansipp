#include <ansipp.hpp>
#include <thread>
#include <algorithm>
#include <cwchar>

using namespace ansipp;

class logo {

    std::string_view logo_text = R"(
▄▀█ █▄ █ █▀ █ █▀█ █▀█
█▀█ █ ▀█ ▄█ █ █▀▀ █▀▀
)";

    unsigned int frame;
    unsigned int width;
    unsigned int height;
    unsigned int move_height;
public:
    logo(): frame(0), width(0), height(0), move_height(0) {
        logo_text.remove_prefix(1);
        
        const char *p = logo_text.data(), *e = p + logo_text.size();
        unsigned int line_width = 0;
        while (p != e) {
            if (*p == '\n') {
                ++p;
                ++height;
                width = std::max(line_width, width);
                line_width = 0;
            } else {
                p += std::mblen(p, e - p);
                ++line_width;
            }
        }
    }

    void process() {
        if (++frame > 1000) {
            frame = 0;
        }
    }

    void draw(ansipp::charbuf& out) {
        out << move(ansipp::move_mode::CURSOR_UP, move_height) << logo_text << charbuf::to_stdout;
        move_height = height;
    }
};

int main() {
    std::locale::global(std::locale(""));

    init_or_exit(ansipp::config { .hide_cursor = true });

    charbuf out;

    logo l;
    while (true) {
        l.process();
        l.draw(out);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    return 0;
}