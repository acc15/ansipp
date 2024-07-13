#include <iostream>
#include <thread>
#include <vector>
#include <cstdlib>

#include <ansipp.hpp>

using namespace ansipp;

struct dead_pixel {
    constexpr static unsigned int max_frame = 256 * 2;

    unsigned int x, y, frame;

    bool process() {
        ++frame;
        return frame < max_frame;
    }

    void draw(std::ostream& out) const {
        unsigned char v = frame < 256 
            ? static_cast<unsigned char>(frame) 
            : static_cast<unsigned char>(256 - (frame - 256)); 
        out << move(x, y) << attrs().bg(rgb { v, v, v }).fg(BLACK) << ' ' << attrs();
    }

};

struct dead_pixels {
    unsigned int next_frames = 10;

    std::vector<dead_pixel> pixels;
    terminal_dimension dim;

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

        next_frames = 10;
        pixels.push_back(dead_pixel{ 
            static_cast<unsigned int>(std::rand() % dim.cols), 
            static_cast<unsigned int>(std::rand() % dim.rows), 
            0 
        });
    }

    void draw(std::ostream& out) const {
        for (const dead_pixel& p: pixels) {
            p.draw(out);
        }
    }

    void loop(std::ostream& out) {
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
    if (std::error_code ec; init(ec), ec) {
        std::cerr << "can't init: " << ec.message() << std::endl;
        return EXIT_FAILURE;
    }

    dead_pixels g;
    terminal_stream out;
    out << hide_cursor();
    g.loop(out);
    return 0;
}
