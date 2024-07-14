#include <iostream>
#include <thread>
#include <atomic>
#include <algorithm>
#include <vector>
#include <deque>
#include <csignal>
#include <cstdlib>
#include <sstream>
#include <chrono>

#include <ansipp.hpp>
#include <unordered_map>

using namespace ansipp;

enum class object_type: unsigned char { EMPTY, APPLE, SNAKE };
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

struct cell {
    object_type type : 2;
    direction dir : 2;
};

enum class apple_type { SMALL, BIG };
struct apple {
    vec pos;
    unsigned int ttl; // in frame count
    apple_type type;
};

template <typename Collection, typename Predicate>
void erase_if(Collection& coll, Predicate pred) {
    auto it = coll.begin();
    while (it != coll.end()) {
        if (pred(*it)) it = coll.erase(it); else ++it;
    }
}

bool remove_prefix(std::string_view& view, std::string_view prefix) {
    if (!view.starts_with(prefix)) return false;
    view.remove_prefix(prefix.size());
    return true;
}

std::string make_border_str(unsigned int width, std::string_view indicators) {
    std::string result(width, ' ');
    return result.replace(1, std::min(result.size(), indicators.size()), indicators);
}

class snake_game {
public:
    static constexpr vec grid_size = { 120, 40 };
    static constexpr vec border_size = grid_size + 2;
    static constexpr vec min_terminal_size = { border_size.x, border_size.y + 1}; // + 1 for blank line with cursor
    static constexpr unsigned int initial_snake_length = 5; 

    cell grid[grid_size.y][grid_size.x] = {};

    bool game_over = false;
    direction dir = direction::RIGHT;
    
    vec tail = { 0, 0 };
    vec head = { initial_snake_length - 1, 0 };
    unsigned int length = initial_snake_length;
    unsigned int frames_till_apple = 0;
    unsigned int grow_frames = 0;
    std::vector<apple> apples;
    unsigned int draw_rows = 0;

    using hr_clock = std::chrono::high_resolution_clock;
    hr_clock::duration last_frame_duration;
    
    char input_buffer[512];
    std::deque<direction> input_queue;

    snake_game() {
        for (unsigned int i = 0; i < initial_snake_length; i++) {
            grid[0][i] = cell { object_type::SNAKE, direction::RIGHT };
        }
    }

    const cell& grid_cell(const vec& v) const { return grid[v.y][v.x]; }
    cell& grid_cell(const vec& v) { return grid[v.y][v.x]; }

    static const char* get_snake_symbol(direction p, direction n, bool head) {
        using enum direction;
        if (head) {
            return "●";
        }

        if ((p == LEFT  || p == RIGHT)  && (n == LEFT   || n == RIGHT)) return "━";
        if ((p == UP    || p == DOWN)   && (n == UP     || n == DOWN))  return "┃";
        if ((p == RIGHT && n == UP)     || (p == DOWN   && n == LEFT))  return "┛";
        if ((p == RIGHT && n == DOWN)   || (p == UP     && n == LEFT))  return "┓";
        if ((p == LEFT  && n == UP)     || (p == DOWN   && n == RIGHT)) return "┗";
        return "┏";
    }

    void process_apples() {

        erase_if(apples, [this](apple& a) {
            if (--a.ttl > 0) return false;
            grid_cell(a.pos).type = object_type::EMPTY;
            return true;
        });

        if (frames_till_apple > 0) {
            --frames_till_apple;
            return;
        }

        frames_till_apple = 10 + std::rand() % 91;
        while (true) {
            vec pos = { std::rand() % grid_size.x, std::rand() % grid_size.y };
            auto& cell = grid_cell(pos);
            if (cell.type == object_type::EMPTY) {
                cell.type = object_type::APPLE;
                apples.emplace_back(apple { 
                    pos, 
                    static_cast<unsigned int>(100 + std::rand() % 900), 
                    static_cast<apple_type>(std::rand() % 2) 
                });
                break;
            }
        }
        
    }

    void process_grow() {
        if (grow_frames > 0) {
            --grow_frames;
            ++length;
            return;
        }
        cell& otc = grid_cell(tail);
        otc.type = object_type::EMPTY;
        tail += dir_to_vec(otc.dir);
    }

    void queue_dir(direction input_dir) {
        direction last_dir = input_queue.empty() ? dir : input_queue.back();
        if (input_dir != last_dir) {
            input_queue.push_back(input_dir);
        }
    }

    bool input() {
        std::string_view rd;
        if (!terminal_read(input_buffer, rd, 0)) { 
            std::cerr << "can't read stdin: " << last_error().message() << std::endl; 
            return false;
        }
        while (!rd.empty()) {
            if      (remove_prefix(rd, "q")) return false;
            else if (remove_prefix(rd, "\33" "[A")) queue_dir(direction::UP);
            else if (remove_prefix(rd, "\33" "[B")) queue_dir(direction::DOWN);
            else if (remove_prefix(rd, "\33" "[C")) queue_dir(direction::RIGHT);
            else if (remove_prefix(rd, "\33" "[D")) queue_dir(direction::LEFT);
            else rd.remove_prefix(1);
        }
        return true;
    }

    void process() {
        if (game_over || draw_rows != static_cast<unsigned int>(grid_size.y + 2)) { 
            return; 
        }

        if (!input_queue.empty()) {
            dir = input_queue.front();
            input_queue.pop_front();
        }
        
        vec next_head = head + dir_to_vec(dir);
        if (next_head.x < 0 || next_head.x >= grid_size.x || next_head.y < 0 || next_head.y >= grid_size.y) {
            game_over = true;
            return;
        }

        cell& nhc = grid_cell(next_head);
        if (nhc.type == object_type::SNAKE) {
            game_over = true;
            return;
        }

        grid_cell(head).dir = dir;
        head = next_head;

        if (nhc.type == object_type::APPLE) {
            const auto it = std::find_if(apples.begin(), apples.end(), 
                [this](const apple& a) { return a.pos == head; });
            if (it != apples.end()) {
                grow_frames += it->type == apple_type::SMALL ? 1 : 5;
                apples.erase(it);
            }
        }

        nhc = cell { object_type::SNAKE, dir };

        process_grow();
        process_apples();
    }

    void draw_border(std::ostream& o) const {
        o   << attrs().bg(WHITE).fg(BLACK)
            << make_border_str(border_size.x, "press q to exit, <arrows> to move") 
            << attrs() << '\n';
                
        for (int y = 0; y < grid_size.y; y++) {
            o   << attrs().bg(WHITE) << " " << attrs() 
                << move(CURSOR_TO_COLUMN, grid_size.x + 2) 
                << attrs().bg(WHITE) << " " << attrs() 
                << '\n';
        }

        o   << attrs().bg(WHITE).fg(BLACK) 
            << make_border_str(border_size.x, (std::stringstream() 
                << "frame = " << std::chrono::duration_cast<std::chrono::microseconds>(last_frame_duration).count()
                << " head = " << head
                << " tail = " << tail
                << " game_over = " << game_over 
                << " apples = " << apples.size()
                << " length = " << length
            ).view())
            << attrs() << '\n';

        o   << move(CURSOR_UP, grid_size.y + 1)
            << move(CURSOR_TO_COLUMN, 2);
    }

    void draw_snake(std::ostream& o) const {
        o   << store_cursor() 
            << move_rel(tail)
            << attrs().fg(color::GREEN);

        vec cur = tail;
        const cell* cell = &grid_cell(cur);
        direction prev_dir = cell->dir;
        while (cur != head) {
            direction next_dir = cell->dir;
            vec v = dir_to_vec(next_dir);
            o   << get_snake_symbol(prev_dir, next_dir, false) 
                << move_rel(v.x - 1, v.y);
            cur += v;
            cell = &grid_cell(cur);
            prev_dir = next_dir;
        }
        o << get_snake_symbol(prev_dir, cell->dir, true) << attrs() << restore_cursor();
    }

    void draw_apples(std::ostream& o) const {
        o << store_cursor() << attrs().fg(color::RED);
        for (const apple& apple: apples) {
            o   << store_cursor() 
                << move_rel(apple.pos)
                << (apple.type == apple_type::SMALL ? "•" : "●")
                << restore_cursor();
        }
        o << attrs();
    }

    void draw(std::ostream& o) {

        o << move(CURSOR_UP, draw_rows) << move(CURSOR_TO_COLUMN, 0) << erase(SCREEN, TO_END);
 
        vec td = get_terminal_dimension();
        if (td.y < min_terminal_size.y || td.x < min_terminal_size.x) {
            o   << "not enough room to render game, current size " << td 
                << " required " << min_terminal_size << '\n' << std::flush;
            draw_rows = 1;
            return;
        }

        draw_border(o);
        draw_snake(o);
        draw_apples(o);
        if (game_over) {
            static const std::string game_over_text =  "GAME IS OVER";
            o   << store_cursor() 
                << move_rel(
                    align(CENTER, grid_size.x, static_cast<int>(game_over_text.size())), 
                    align(CENTER, grid_size.y, 1)
                )
                << attrs().bg(WHITE).fg(BLACK) << game_over_text << attrs() << restore_cursor();
        }
        o << move(CURSOR_DOWN, grid_size.y + 1) << move(CURSOR_TO_COLUMN, 0)  << std::flush;
        draw_rows = border_size.y;
    }

    void loop(std::ostream& out) {
        draw(out);
        while (!game_over) {
            std::this_thread::sleep_for(std::chrono::milliseconds(80));
            hr_clock::time_point t1 = hr_clock::now();
            if (!input()) break;
            process();
            draw(out);
            last_frame_duration = hr_clock::now() - t1;
        }
    }

};

int main() {
    const config c = {
        .hide_cursor = true,

    };
    if (std::error_code ec; init(ec, c), ec) {
        std::cerr << "can't init: " << ec.message() << std::endl;
        return EXIT_FAILURE;
    }

    snake_game g;

    terminal_stream out;
    g.loop(out);
    return 0;
}
