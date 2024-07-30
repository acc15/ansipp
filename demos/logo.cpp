#include <ansipp.hpp>
#include <thread>


class logo {

    std::string_view logo_text = R"(
▄▀█ █▄░█ █▀ █ █▀█ █▀█
█▀█ █░▀█ ▄█ █ █▀▀ █▀▀
)";
    unsigned int frame;
public:
    logo() {
        logo_text.remove_prefix(1);
    }

    void process() {
        if (++frame > 1000) {
            frame = 0;
        }
    }

    void draw(ansipp::charbuf& out) {
        out << logo_text << ansipp::move(ansipp::move_mode::CURSOR_UP, 2) << ansipp::charbuf::to_stdout;
    }
};

int main() {
    ansipp::init_or_exit(ansipp::config { .hide_cursor = true });

    ansipp::charbuf out;

    logo l;
    while (true) {
        l.process();
        l.draw(out);
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
    }
    return 0;
}