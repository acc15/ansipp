#include <iostream>
#include <thread>
#include <atomic>
#include <csignal>

#include <ansipp.hpp>
#include <unordered_map>

using namespace ansipp;

struct vec { 
    int x;
    int y; 
    vec& operator+=(const vec& b) { x += b.x; y += b.y; return *this; }
    vec operator+(const vec& b) const { return vec{x,y} += b; }
    bool operator==(const vec& b) const { return x == b.x && y == b.y; }
};

enum class object_type: unsigned char {
    EMPTY,
    APPLE,
    SNAKE
};

enum class direction: unsigned char { UP, DOWN, LEFT, RIGHT };
vec dir_to_vec(direction d) {
    switch (d) {
        using enum direction;
        case UP:    return {  0, -1 };
        case DOWN:  return {  0,  1 };
        case LEFT:  return { -1,  0 };
        case RIGHT: return {  1,  0 };
        default:    return vec{};
    }
}

struct cell {
    object_type type : 2;
    direction prev_dir : 2;
    direction next_dir : 2;
};

class snake_game {

public:
    static constexpr vec grid_size = { 120, 40 };
    static constexpr unsigned int initial_length = 20;

    cell grid[grid_size.y][grid_size.x] = {};
    direction prev_dir = dir;
    vec tail = { 0, 0 };
    vec head = { initial_length - 1, 0 };

    std::atomic<direction> dir = direction::RIGHT;
    std::atomic_bool game_over = false;

    snake_game() {
        for (unsigned int i = 0; i < initial_length; i++) {
            grid[0][i] = cell { object_type::SNAKE, direction::RIGHT, direction::RIGHT };
        }
    }

    const cell& grid_cell(const vec& v) const { return grid[v.y][v.x]; }
    cell& grid_cell(const vec& v) { return grid[v.y][v.x]; }

    void draw_cell(std::ostream& o, const vec& v) const {
        const cell& c = grid_cell(v);
        switch (c.type) {
            case object_type::EMPTY: 
                o << " "; 
                return;

            case object_type::APPLE: 
                o << attrs().fg(color::RED) << "" << attrs();
                return;

            default: break;
        }

        const direction p = c.prev_dir, d = c.next_dir;
        using enum direction;

        o << attrs().fg(color::GREEN);
        if (v == head)                                                      o << "";
        else if ((p == LEFT || p == RIGHT) && (d == LEFT || d == RIGHT))    o << "━";
        else if ((p == UP || p == DOWN) && (d == UP || d == DOWN))          o << "┃";
        else if ((p == RIGHT && d == UP) || (p == DOWN && d == LEFT))       o << "┛";
        else if ((p == RIGHT && d == DOWN) || (p == UP && d == LEFT))       o << "┓";
        else if ((p == LEFT && d == UP) || (p == DOWN && d == RIGHT))       o << "┗";
        else if ((p == LEFT && d == DOWN) || (p == UP && d == RIGHT))       o << "┏";
        o << attrs();
    }

    void process() {
        if (game_over) { 
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

        cell& ohc = grid_cell(head);
        head = next_head;
        ohc.next_dir = prev_dir = dir;

        if (nhc.type == object_type::EMPTY) {
            cell& otc = grid_cell(tail);
            otc.type = object_type::EMPTY;
            tail += dir_to_vec(otc.next_dir);
            cell& ntc = grid_cell(tail);
            ntc.prev_dir = ntc.next_dir;
        }
        nhc = cell { object_type::SNAKE, dir, dir };

    }

    void draw(std::ostream& o) const {
        for (int x = 0; x < grid_size.x + 2; x++) { o << "█"; }
        o << "\n";
        
        for (int y = 0; y < grid_size.y; y++) {
            o << "█";
            for (int x = 0; x < grid_size.x; x++) { 
                draw_cell(o, {x,y}); 
            }
            o << "█";
            o << "\n";
        }
        
        for (int x = 0; x < grid_size.x + 2; x++) { std::cout << "█"; }
        o << "\n";
        o << erase(LINE, TO_END)
             << "head = " << head.x << "," << head.y 
             << "; tail = " << tail.x << "," << tail.y 
             << "; game_over = " << game_over << "\n";
        o << std::flush;
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

    std::jthread t(input_thread, std::ref(g));

    g.draw(std::cout);
    while (!g.game_over) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        g.process();
        std::cout << move(CURSOR_UP, g.grid_size.y + 3);
        g.draw(std::cout);
    }
    t.join();
    return 0;
}
