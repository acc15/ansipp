#include <ansipp.hpp>
#include <thread>
#include <algorithm>
#include <cwchar>
#include <cstdlib>
#include <vector>
#include <string>

using namespace ansipp;

vec compute_mbstr_dim(std::string_view str) {
    vec result;
    int line_width = 0;
    for (const char *p = str.data(), *e = p + str.size(); p != e; p += std::mblen(p, e - p)) {
        if (*p != '\n') {
            ++line_width;
            continue;
        }
        result.x = std::max(line_width, result.x);
        ++result.y;
        line_width = 0;
    }
    return result;
}


class ctrl;

class c_logo {
    static constexpr int gradient_width = 10;
    static constexpr std::string_view str = R"(
▄▀█ █▄ █ █▀ █ █▀█ █▀█
█▀█ █ ▀█ ▄█ █ █▀▀ █▀▀
)";
    int frame = 0;
public:
    const vec dim = compute_mbstr_dim(str);
    rgb pos_to_rgb(vec v) const;
    void process(ctrl& c);
    void draw(const ctrl& c) const;
};

struct c_escape {
    std::string str;
    vec pos;
    rgb color;
    bool process(ctrl& c);
    void draw(const ctrl& c) const;
};

struct c_terminal {
    static constexpr int glow_duration = 40;
    static constexpr rgb glow_color = 0xffffff;
    const vec dim = vec(6, 3);
    int glow = 0;
    rgb color = 0x00ff00;
    void process(ctrl& c);
    void draw(const ctrl& c) const;
};

class ctrl {
    unsigned int erase_height = 0;

public:
    constexpr static int screen_width = 80;

    mutable charbuf out;

    c_logo logo;
    c_terminal terminal;
    std::vector<c_escape> escapes;

    void add_escape(c_escape&& v) { escapes.push_back(std::move(v)); }
    void process() {
        terminal.process(*this);
        std::erase_if(escapes, [this](c_escape& e) { return !e.process(*this); });
        logo.process(*this);
        erase_height = logo.dim.y;
    }

    void draw() const {
        out << move(move_mode::CURSOR_UP_START, erase_height) << erase(SCREEN, TO_END); 
        for (const c_escape& e: escapes) e.draw(*this);
        logo.draw(*this);
        terminal.draw(*this);
        out << move(CURSOR_DOWN_START, logo.dim.y) << charbuf::to_stdout;
    }

};

void c_logo::process(ctrl& c) {
    // std::erase_if(sequences, [this](escape_sequence& seq) { return process_seq(seq); });
    if (++frame >= dim.x * 4) frame = 0;
    if (frame - gradient_width == dim.x) {
        rgb color(
            0x80 + std::rand() % 0x80, 
            0x80 + std::rand() % 0x80, 
            0x80 + std::rand() % 0x80
        );
        std::string esc_suffix = esc_str(attrs().fg(color)).erase(0, 1);
        c.add_escape(c_escape { .str = std::move(esc_suffix), .pos = vec(dim.x, std::rand() % dim.y), .color = color });
    }
}

rgb c_logo::pos_to_rgb(vec v) const {
    const rgb start(255, 255, 255); 
    const rgb end(100, 100, 100);
    
    int center = frame - gradient_width;
    float factor = static_cast<float>(std::min(gradient_width, std::abs(center - v.x))) / gradient_width;
    return rgb::lerp(start, end, factor);
}

void c_logo::draw(const ctrl& c) const {
    vec v;
    c.out << store_cursor;
    for (const char *p = str.data(), *e = p + str.size(); p != e;) {
        int cw = std::mblen(p, e - p);
        if (*p == '\n') {
            c.out << *p;
            v.x = 0;
            ++v.y;
        } else {
            c.out << attrs().fg(pos_to_rgb(v)) << std::string_view(p, cw) << attrs();
            ++v.x;
        }
        p += cw;
    }
    c.out << restore_cursor;
}

bool c_escape::process(ctrl& c) {
    ++pos.x;
    if (pos.x < ctrl::screen_width - c.terminal.dim.x - static_cast<int>(str.size() + 4)) return true;
    c.terminal.glow = c.terminal.glow_duration;
    c.terminal.color = color;
    return false;
}

void c_escape::draw(const ctrl& c) const {
    c.out << store_cursor << move_rel(pos) << "\x1b" << str << "\\x1b" << str << attrs() << restore_cursor;
}

void c_terminal::process(ctrl&) {
    if (glow > 0) --glow;
}

void c_terminal::draw(const ctrl& c) const {
    const int x = c.screen_width - dim.x;
    rgb bg_color = glow <= 0 ? 0x202020 : rgb::lerp(0x202020, glow_color, static_cast<float>(glow) / glow_duration);
    c.out
        << store_cursor
        << move(CURSOR_TO_COLUMN, x)
        << "╭────╮"
        << move(CURSOR_TO_COLUMN, x) << move(CURSOR_DOWN)
        << "│" << attrs().on(BOLD).bg(bg_color).fg(color) << " >_ " << attrs() << "│"
        << move(CURSOR_TO_COLUMN, x) << move(CURSOR_DOWN)
        << "╰────╯"
        << restore_cursor;
}

int main() {
    std::locale::global(std::locale(""));
    init_or_exit(ansipp::config { .hide_cursor = true });

    ctrl c;
    while (true) {
        c.draw();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        c.process();
    }
    return 0;
}