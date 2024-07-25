#include <iostream>
#include <thread>
#include <algorithm>
#include <vector>
#include <deque>
#include <chrono>
#include <limits>

#include <ansipp.hpp>
#include <unordered_map>

using namespace ansipp;

enum class direction: unsigned char { UP, DOWN, LEFT, RIGHT };
vec dir_to_vec(direction d) {
    using enum direction;
    switch (d) {
        default:
        case UP:    return {  0, -1 };
        case DOWN:  return {  0,  1 };
        case LEFT:  return { -1,  0 };
        case RIGHT: return {  1,  0 };
    }
}

template <>
struct std::hash<vec> {
    std::size_t operator()(const vec& v) const {
        return std::hash<int>{}(v.x) ^ std::rotl(std::hash<int>{}(v.y), std::numeric_limits<std::size_t>::digits/2);
    }
};

enum class apple_type { SMALL, BIG };
struct apple {
    unsigned int ttl; // in frame count
    apple_type type;
};

bool remove_prefix(std::string_view& view, std::string_view prefix) {
    if (!view.starts_with(prefix)) return false;
    view.remove_prefix(prefix.size());
    return true;
}

class snake_game {
public:
    static constexpr vec grid_size = { 120, 40 };
    static constexpr vec border_size = grid_size + vec(2);
    static constexpr vec min_terminal_size = { border_size.x, border_size.y + 1}; // + 1 for blank line with cursor
    static constexpr unsigned int initial_snake_length = 5; 

    using snake_map = std::unordered_map<vec, direction>;
    using apple_map = std::unordered_map<vec, apple>;

    snake_map snake;
    apple_map apples;

    bool initializing = true;
    bool game_over = false;
    direction dir = direction::RIGHT;
    
    vec tail = { 0, 0 };
    vec head = { initial_snake_length - 1, 0 };
    vec terminal_size = {};
    bool undersize = false;
    bool paused = false;

    unsigned int length = initial_snake_length;
    unsigned int frames_till_apple = 0;
    unsigned int grow_frames = 0;
    unsigned int draw_rows = 0;

    using hr_clock = std::chrono::high_resolution_clock;
    /*std::size_t input_ns;
    std::size_t process_ns;
    std::size_t draw_ns;
    std::size_t print_ns;*/
    std::size_t frame_ns;
    
    charbuf out;
    char input_buffer[512];
    std::deque<direction> input_queue;

    snake_game(): out(4096) {
        for (unsigned int i = 0; i < initial_snake_length; i++) {
            snake[vec(i, 0)] = direction::RIGHT;
        }
    }

    static const char* get_snake_symbol(direction p, direction n) {
        using enum direction;
        if ((p == LEFT  || p == RIGHT)  && (n == LEFT   || n == RIGHT)) return "━";
        if ((p == UP    || p == DOWN)   && (n == UP     || n == DOWN))  return "┃";
        if ((p == RIGHT && n == UP)     || (p == DOWN   && n == LEFT))  return "┛";
        if ((p == RIGHT && n == DOWN)   || (p == UP     && n == LEFT))  return "┓";
        if ((p == LEFT  && n == UP)     || (p == DOWN   && n == RIGHT)) return "┗";
        return "┏";
    }

    vec rnd_vec() const {
        return vec { std::rand() % grid_size.x, std::rand() % grid_size.y };
    }

    void process_apples() {
        std::erase_if(apples, [](apple_map::value_type& a) { return --a.second.ttl == 0; });
        if (frames_till_apple > 0) {
            --frames_till_apple;
            return;
        }

        frames_till_apple = 10 + std::rand() % 91;

        vec pos = rnd_vec();
        for (; snake.contains(pos) || apples.contains(pos); pos = rnd_vec());
        apples[pos] = apple {
            static_cast<unsigned int>(100 + std::rand() % 900),
            static_cast<apple_type>(std::rand() % 2)
        };
    }

    void queue_dir(direction input_dir) {
        if (paused) return;
        direction last_dir = input_queue.empty() ? dir : input_queue.back();
        if (input_dir != last_dir) input_queue.push_back(input_dir);
    }

    bool input() {
        std::string_view rd;
        do {
            if (!terminal_read(input_buffer, rd, 0)) { 
                std::cerr << "can't read stdin: " << last_error().message() << std::endl; 
                return false;
            }
            if (rd.empty()) break;

            std::string_view remaining = rd;
            while (!remaining.empty()) {
                if (remove_prefix(remaining, "q")) return false;
                if (remove_prefix(remaining, " ")) { if (!undersize) paused = !paused; continue; }
                if (remove_prefix(remaining, "\33" "[A")) { queue_dir(direction::UP); continue; };
                if (remove_prefix(remaining, "\33" "[B")) { queue_dir(direction::DOWN); continue; }
                if (remove_prefix(remaining, "\33" "[C")) { queue_dir(direction::RIGHT); continue; }
                if (remove_prefix(remaining, "\33" "[D")) { queue_dir(direction::LEFT); continue; }
                remaining.remove_prefix(1); // drop unhandled char
            }
        } while (rd.size() == std::size(input_buffer));
        return true;
    }

    void process() {
        terminal_size = get_terminal_size();
        undersize = terminal_size.y < min_terminal_size.y || terminal_size.x < min_terminal_size.x;
        if (undersize) paused = true;
        if (initializing || paused) return;

        if (!input_queue.empty()) {
            dir = input_queue.front();
            input_queue.pop_front();
        }
        
        vec next_head = head + dir_to_vec(dir);
        if (next_head.x < 0 || next_head.x >= grid_size.x ||
            next_head.y < 0 || next_head.y >= grid_size.y ||
            snake.contains(next_head))
        {
            game_over = true;
            return;
        }

        snake[head] = dir;
        head = next_head;

        if (apples.contains(head)) {
            grow_frames += apples[head].type == apple_type::SMALL ? 1 : 5;
            apples.erase(head);
        }

        snake[head] = dir;
        if (grow_frames > 0) {
            --grow_frames;
            ++length;
        } else {
            direction tail_dir = snake[tail];
            snake.erase(tail);
            tail += dir_to_vec(tail_dir);
        }
        process_apples();
    }

    void draw_border() {
        out << attrs().bg(WHITE).fg(BLACK);

        std::size_t top_offset = out.size();
        out << " press q to exit, <arrows> to move, <space> to pause/unpause";
        out.put(' ', border_size.x - (out.size() - top_offset));

        out << attrs() << '\n';

        for (int y = 0; y < grid_size.y; y++) {
            out << attrs().bg(WHITE) << ' ' << attrs() << move(CURSOR_TO_COLUMN, border_size.x);
            out << attrs().bg(WHITE) << ' ' << attrs() << '\n';
        }

        out << attrs().bg(WHITE).fg(BLACK);
        
        std::size_t bottom_offset = out.size();
        out /*<< " input=" << input_ns
            << " process=" << process_ns
            << " draw=" << draw_ns
            << " print=" << print_ns*/
            << " frame=" << frame_ns
            << " head=" << head
            << " tail=" << tail
            << " game_over=" << static_cast<unsigned int>(game_over)
            << " apples=" << apples.size()
            << " length=" << length;
        out.put(' ', border_size.x - (out.size() - bottom_offset));
        
        out << attrs() << '\n' << move(CURSOR_UP, grid_size.y + 1) << move(CURSOR_TO_COLUMN, 2);
    }

    void draw_snake() {
        out << store_cursor << move_rel(tail) << attrs().fg(color::GREEN);

        direction prev_dir = snake.at(tail);
        for (vec cur = tail; cur != head;) {
            direction next_dir = snake.at(cur);
            vec v = dir_to_vec(next_dir);
            out << get_snake_symbol(prev_dir, next_dir) << move_rel(v.x - 1, v.y);
            cur += v;
            prev_dir = next_dir;
        }
        out << "●" << attrs() << restore_cursor;
    }

    void draw_apples() {
        out << attrs().fg(color::RED);
        for (const auto& apple: apples) {
            out << store_cursor
                << move_rel(apple.first)
                << (apple.second.type == apple_type::SMALL ? "•" : "●")
                << restore_cursor;
        }
        out << attrs();
    }

    void draw_center_text(std::string_view str) {
        out << store_cursor
            << move_rel(align(CENTER, grid_size.x, static_cast<int>(str.size())), align(CENTER, grid_size.y, 1))
            << attrs().bg(WHITE).fg(BLACK) << str << attrs() 
            << restore_cursor;
    }

    void draw() {
        out << move(CURSOR_TO_COLUMN, 0) << move(CURSOR_UP, draw_rows) << erase(SCREEN, TO_END);
        if (undersize) {
            out << "not enough room to render game, current size " 
                << terminal_size << " required " << min_terminal_size << '\n';
            draw_rows = 1;
        } else {
            draw_border();
            draw_snake();
            draw_apples();
            if (game_over) {
                draw_center_text("GAME IS OVER");
            } else if (paused) {
                draw_center_text("PAUSE");
            }
            out << move(CURSOR_DOWN, grid_size.y + 1) << move(CURSOR_TO_COLUMN, 0);
            draw_rows = border_size.y;
        }
        terminal_write(out.flush());
    }

    void loop() {
        while (true) {
            hr_clock::time_point t1 = hr_clock::now();
            if (!input()) break;
            // hr_clock::time_point t2 = hr_clock::now();
            process();
            // hr_clock::time_point t3 = hr_clock::now();
            draw();
            // hr_clock::time_point t4 = hr_clock::now();
            hr_clock::time_point t5 = hr_clock::now();

            // input_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
            // process_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t3 - t2).count();
            // draw_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t4 - t3).count();
            // print_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t5 - t4).count();
            frame_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t5 - t1).count();

            initializing = false;

            if (game_over) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(80));
        }
    }

};

int main() {
    if (std::error_code ec; init(ec, config { .disable_input_signal = true, .hide_cursor = true }), ec) {
        std::cerr << "can't init: " << ec.message() << std::endl;
        return EXIT_FAILURE;
    }
    snake_game().loop();
    return 0;
}
