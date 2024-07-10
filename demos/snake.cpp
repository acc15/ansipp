#include <iostream>
#include <thread>
#include <atomic>
#include <algorithm>
#include <vector>
#include <csignal>
#include <cstdlib>
#include <sstream>

#include <ansipp.hpp>
#include <unordered_map>

using namespace ansipp;

struct vec { 
    int x;
    int y; 
    constexpr vec& operator+=(const vec& b) { x += b.x; y += b.y; return *this; }
    constexpr vec& operator+=(const int& v) { x += v; y += v; return *this; }
    constexpr vec operator+(const vec& b) const { return vec{x,y} += b; }
    constexpr vec operator+(const int& b) const { return vec{x,y} += b; }
    constexpr bool operator==(const vec& b) const { return x == b.x && y == b.y; }
};
std::ostream& operator<<(std::ostream& o, const vec& v) { return o << v.x << "," << v.y; }

enum class object_type: unsigned char { EMPTY, APPLE, SNAKE };
enum class direction: unsigned char { UP, DOWN, LEFT, RIGHT };

vec dir_to_vec(direction d) {
    switch (d) {
        using enum direction;
        default:    return {  0, -1 };
        case DOWN:  return {  0,  1 };
        case LEFT:  return { -1,  0 };
        case RIGHT: return {  1,  0 };
    }
}

move_mode dir_to_cursor_move(direction d) { 
    switch (d) {
        using enum direction;
        default:    return CURSOR_UP;
        case DOWN:  return CURSOR_DOWN;
        case LEFT:  return CURSOR_LEFT;
        case RIGHT: return CURSOR_RIGHT;
    }
}

struct cell {
    object_type type : 2;
    direction dir : 2;
};

enum class apple_type { SMALL, BIG };
struct apple {
    vec pos;
    std::size_t ttl;
    apple_type type;
};

template <typename Collection, typename Predicate>
void erase_if(Collection& coll, Predicate pred) {
    auto it = coll.begin();
    while (it != coll.end()) {
        if (pred(*it)) it = coll.erase(it); else ++it;
    }
}

const std::string game_over_text =  "GAME IS OVER";
const std::string snake_head =      "";
const std::string snake_tail_h =    "━";
const std::string snake_tail_v =    "┃";
const std::string snake_tail_ru =   "┛";
const std::string snake_tail_rd =   "┓";
const std::string snake_tail_lu =   "┗";
const std::string snake_tail_ld =   "┏";
const std::string apple_small =     "•";
const std::string apple_big =       "";

class snake_game {
public:
    static constexpr vec grid_size = { 120, 40 };
    static constexpr vec border_size = grid_size + 2;
    static constexpr unsigned int initial_snake_length = 5;

    cell grid[grid_size.y][grid_size.x] = {};

    std::atomic_bool game_over = false;
    std::atomic<direction> dir = direction::RIGHT;
    
    vec tail = { 0, 0 };
    vec head = { initial_snake_length - 1, 0 };
    unsigned int length = initial_snake_length;
    unsigned int frames_till_apple = 0;
    unsigned int grow_frames = 0;
    std::vector<apple> apples;
    unsigned int draw_height = 0;

    snake_game() {
        for (unsigned int i = 0; i < initial_snake_length; i++) {
            grid[0][i] = cell { object_type::SNAKE, direction::RIGHT };
        }
    }

    const cell& grid_cell(const vec& v) const { return grid[v.y][v.x]; }
    cell& grid_cell(const vec& v) { return grid[v.y][v.x]; }

    const std::string& get_snake_tail(direction p, direction n) const {
        using enum direction;
        if ((p == LEFT  || p == RIGHT)  && (n == LEFT   || n == RIGHT)) return snake_tail_h;
        if ((p == UP    || p == DOWN)   && (n == UP     || n == DOWN))  return snake_tail_v;
        if ((p == RIGHT && n == UP)     || (p == DOWN   && n == LEFT))  return snake_tail_ru;
        if ((p == RIGHT && n == DOWN)   || (p == UP     && n == LEFT))  return snake_tail_rd;
        if ((p == LEFT  && n == UP)     || (p == DOWN   && n == RIGHT)) return snake_tail_lu;
        return snake_tail_ld;
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

    void process() {
        if (game_over || draw_height != static_cast<unsigned int>(grid_size.y + 2)) { 
            return; 
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
        o << attrs().bg(color::WHITE) << std::string(border_size.x, ' ') << attrs() << '\n';
        for (int y = 0; y < grid_size.y; y++) {
            o   << attrs().bg(color::WHITE) << " " << attrs() 
                << move(CURSOR_TO_COLUMN, grid_size.x + 2) 
                << attrs().bg(color::WHITE) << " " << attrs() 
                << '\n';
        }
        
        std::stringstream status_line_stream;
        status_line_stream
            << "head = " << head
            << " tail = " << tail
            << " game_over = " << game_over 
            << " apples = " << apples.size()
            << " length = " << length;

        std::string_view status_line = status_line_stream.view();
        std::string bottom_border = std::string(border_size.x, ' ');

        o   << attrs().bg(color::WHITE).fg(color::BLACK) 
            << bottom_border.replace(1, std::min(status_line.size(), bottom_border.size()), status_line) 
            << attrs() << '\n'
            << move(CURSOR_UP, grid_size.y + 1)
            << move(CURSOR_TO_COLUMN, 2);
    }

    void draw_snake(std::ostream& o) const {
        o   << store_cursor() 
            << move_xy(tail.x, tail.y)
            << attrs().fg(color::GREEN);

        vec cur = tail;
        const cell* cell = &grid_cell(cur);
        direction prev_dir = cell->dir;
        while (cur != head) {
            direction next_dir = cell->dir;
            o   << get_snake_tail(prev_dir, next_dir) 
                << move(CURSOR_LEFT) 
                << move(dir_to_cursor_move(next_dir));
            cur += dir_to_vec(next_dir);
            cell = &grid_cell(cur);
            prev_dir = next_dir;
        }
        o << snake_head << attrs() << restore_cursor();
    }

    void draw_apples(std::ostream& o) const {
        o << store_cursor() << attrs().fg(color::RED);
        for (const apple& apple: apples) {
            o   << store_cursor() 
                << move_xy(apple.pos.x, apple.pos.y)
                << (apple.type == apple_type::SMALL ? "•" : "●")
                << restore_cursor();
        }
        o << attrs();
    }

    void draw(std::ostream& o) {
        
        terminal_dimension td = get_terminal_dimension();

        o << move(CURSOR_TO_COLUMN, 0) << move(CURSOR_UP, draw_height) << erase(SCREEN, TO_END);

        if (td.rows < border_size.y || td.cols < border_size.x) {
            o   << "not enough room to render game, current size " << td 
                << " required " << border_size.x << "x" << border_size.y << '\n' << std::flush;
            draw_height = 1;
            return;
        }

        draw_border(o);
        draw_snake(o);
        draw_apples(o);
        if (game_over) {
            o   << store_cursor() 
                << move_xy(
                    align(CENTER, grid_size.x + 2, static_cast<int>(game_over_text.size())), 
                    align(CENTER, grid_size.y + 2, 1)
                    )
                << attrs().bg(WHITE).fg(BLACK) << game_over_text << attrs() << restore_cursor();
        }
        o << move(CURSOR_TO_COLUMN, 0) << move(CURSOR_DOWN, grid_size.y + 1) << std::flush;
        draw_height = grid_size.y + 2;
    }

};

void input_thread(snake_game& game) {
    std::string seq;
    while (!game.game_over && terminal_read(seq) && seq != "q") {
        if      (seq == "\33" "[A") game.dir = direction::UP;
        else if (seq == "\33" "[B") game.dir = direction::DOWN;
        else if (seq == "\33" "[C") game.dir = direction::RIGHT;
        else if (seq == "\33" "[D") game.dir = direction::LEFT;
    }
    game.game_over = true;
}

int main() {

    if (std::error_code ec; init(ec), ec) {
        std::cerr << "can't init: " << ec.message() << std::endl;
        return EXIT_FAILURE;
    }

    snake_game g;
    std::thread t(input_thread, std::ref(g));

    terminal_stream out;
    out << hide_cursor() << disable_line_wrap();

    g.draw(out);
    while (!g.game_over) {
        std::this_thread::sleep_for(std::chrono::milliseconds(80));
        g.process();
        g.draw(out);
    }
    t.join();
    return 0;
}
