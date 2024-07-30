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

};

struct dead_pixels {
    unsigned int next_frames = 10;

    charbuf out = charbuf(4096);
    std::vector<dead_pixel> pixels;
    vec dim;

    void process() {
        std::erase_if(pixels, [](dead_pixel& p) {
            p.frame += 8;
            return p.frame >= p.max_frame;
        });
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

    void draw() {
        out << store_cursor;
        for (const dead_pixel& p: pixels) {
            unsigned char v = p.frame < 256 
                ? static_cast<unsigned char>(p.frame) 
                : static_cast<unsigned char>(255 - (p.frame - 256)); 
            out << move_abs(p.pos) << attrs().bg(rgb { v, v, v }).fg(BLACK) << ' ' << attrs();
        }
        out << restore_cursor << charbuf::to_stdout;
    }

    void loop() {
        dim = get_terminal_size();
        draw();

        char buf[20];
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));

            std::string_view rd;
            if (!stdin_read(buf, rd, 0)) {
                std::cerr << "can't read stdin: " << last_error().message() << std::endl;
                return;
            }
            if (rd.find('q', 0) != std::string_view::npos) {
                break;
            }

            process();
            draw();
        }
    }

};

int main() {
    init_or_exit({ .hide_cursor = true });
    dead_pixels().loop();
    return 0;
}
