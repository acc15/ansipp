#include <iostream>
#include <thread>
#include <vector>
#include <cstdlib>

#include <ansipp.hpp>

using namespace ansipp;

struct dead_pixel {
    constexpr static unsigned int max_frame = 256 * 2;

    vec pos;
    unsigned int frame;

    bool process() {
        frame += 8;
        return frame < max_frame;
    }

    void draw(std::ostream& out) const {
        unsigned char v = frame < 256 
            ? static_cast<unsigned char>(frame) 
            : static_cast<unsigned char>(255 - (frame - 256)); 
        out << move_abs(pos) << attrs().bg(rgb { v, v, v }).fg(BLACK) << ' ' << attrs();
    }

};

struct dead_pixels {
    unsigned int next_frames = 10;

    std::vector<dead_pixel> pixels;
    vec dim;

    void process() {
        auto it = pixels.begin();
        while (it != pixels.end()) {
            if (it->process()) {
                ++it;
            } else {
                it = pixels.erase(it);
            }
        }

        if (next_frames > 0) {
            --next_frames;
            return;
        }

        next_frames = 2;
        pixels.push_back(dead_pixel { 
            .pos = vec(
                static_cast<unsigned int>(std::rand() % dim.x), 
                static_cast<unsigned int>(std::rand() % dim.y)
            ),
            .frame = 0 
        });
    }

    void draw(std::ostream& out) const {
        out << store_cursor;
        for (const dead_pixel& p: pixels) {
            p.draw(out);
        }
        out << restore_cursor << std::flush;
    }

    void loop(std::ostream& out) {
        dim = get_terminal_size();
        draw(out);

        char buf[20];
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));

            std::string_view rd;
            if (!terminal_read(buf, rd, 0)) {
                std::cerr << "can't read stdin: " << last_error().message() << std::endl;
                return;
            }
            if (rd.find('q', 0) != std::string_view::npos) {
                break;
            }

            process();
            draw(out);
        }
    }

};

int main() {
    if (std::error_code ec; init(ec, { .hide_cursor = true }), ec) {
        std::cerr << "can't init: " << ec.message() << std::endl;
        return EXIT_FAILURE;
    }

    dead_pixels g;
    terminal_stream out;
    g.loop(out);
    return 0;
}
