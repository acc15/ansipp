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
    direction dir : 2;
    direction prev_dir : 2;
};

std::string get_symbol(const cell& c) {
    switch (c.type) {
    using enum object_type;
    case EMPTY: return " ";
    case APPLE: return "";
    default: 
        vec po = dir_to_vec(c.prev_dir);
        vec no = dir_to_vec(c.dir);
        if (po.y == 0 && no.y == 0) {
            return "━";
        }
        if (po.x == 0 && no.x == 0) {
            return "┃";
        }
        return "L";
    }
}

// std::unordered_map<object_type, std::string> snake_symbols = {
//     { EMPTY, " " },
//     { SNAKE_HEAD, "⚉" },
//     { SNAKE_LR, "━" },
//     { SNAKE_LU, "┛" },
//     { SNAKE_LD, "┓" },
//     { SNAKE_RL, "━" },
//     { SNAKE_RU, "┗" },
//     { SNAKE_RD, "┏" },
//     { SNAKE_UD, "┃" },
//     { SNAKE_UR, "┗" },
//     { SNAKE_UL, "┛" },
//     { SNAKE_DU, "┃" },
//     { SNAKE_DR, "┏" },
//     { SNAKE_DL, "┓" },
//     { APPLE, "" }
// };

class snake_game {

public:
    static constexpr vec grid_size = { 120, 40 };
    static constexpr unsigned int initial_length = 1;

    cell grid[grid_size.y][grid_size.x] = {};
    direction prev_dir = dir;
    vec tail = { 0, 0 };
    vec head = { initial_length, 0 };

    std::atomic<direction> dir = direction::RIGHT;
    std::atomic_bool game_over = false;

    snake_game() {
        for (unsigned int i = 0; i < initial_length; i++) {
            grid[0][i] = cell { object_type::SNAKE, direction::RIGHT, direction::RIGHT };
        }
    }

    const cell& grid_cell(const vec& v) const { return grid[v.y][v.x]; }
    cell& grid_cell(const vec& v) { return grid[v.y][v.x]; }

    const char* snake_symbol(const vec& v) const {
        const cell& c = grid_cell(v);
        switch (c.type) {
            case object_type::EMPTY: return " ";
            case object_type::APPLE: return "";
            default: break;
        }
        if (v == head) return "";
        const direction p = c.prev_dir, d = c.dir;
        using enum direction;
        if ((p == LEFT || p == RIGHT) && (d == LEFT || d == RIGHT))     return "━";
        if ((p == UP || p == DOWN) && (d == UP || d == DOWN))           return "┃";
        if ((p == RIGHT && d == UP) || (p == DOWN && d == LEFT))        return "┛";
        if ((p == RIGHT && d == DOWN) || (p == UP && d == LEFT))        return "┓";
        if ((p == LEFT && d == UP) || (p == DOWN && d == RIGHT))        return "┗";
        if ((p == LEFT && d == DOWN) || (p == UP && d == RIGHT))        return "┏";
        return "X";
    }

    void process() {
        if (game_over) { return; }

        vec h = head + dir_to_vec(dir);
        if (h.x < 0 || h.x >= grid_size.x || h.y < 0 || h.y >= grid_size.y) {
            game_over = true;
            return;
        }

        cell& nhc = grid_cell(h);
        if (nhc.type == object_type::SNAKE) {
            game_over = true;
            return;
        }

        cell& ohc = grid_cell(head);
        if (nhc.type == object_type::EMPTY) {
            cell& otc = grid_cell(tail);
            otc.type = object_type::EMPTY;
            tail += dir_to_vec(otc.dir);
            cell& ntc = grid_cell(tail);
            ntc.prev_dir = ntc.dir;
        }

        ohc = cell { object_type::SNAKE, dir, prev_dir };
        nhc = cell { object_type::SNAKE, dir, dir };

        head = h;
        prev_dir = dir;
    }

    void draw() const {
        std::cout << restore_position() << move(0, 0);
        for (int x = 0; x < grid_size.x + 2; x++) { std::cout << "█"; }
        std::cout << "\n";
        
        for (int y = 0; y < grid_size.y; y++) {
            std::cout << "█";
            for (int x = 0; x < grid_size.x; x++) { 
                std::cout << snake_symbol({x,y}); 
            }
            std::cout << "█";
            std::cout << "\n";
        }
        
        for (int x = 0; x < grid_size.x + 2; x++) { std::cout << "█"; }
        std::cout << "\n" << std::flush;
    }

};

void input_thread(snake_game& game) {
    std::string seq;
    while (!game.game_over && terminal_read(seq) && seq != "q") {
        if (seq == "\33" "[A") { game.dir = direction::UP; }
        if (seq == "\33" "[B") { game.dir = direction::DOWN; }
        if (seq == "\33" "[C") { game.dir = direction::RIGHT; }
        if (seq == "\33" "[D") { game.dir = direction::LEFT; }
    }
    game.game_over = true;
}

int main() {

    if (std::error_code ec; init(ec), ec) {
        std::cerr << "can't init: " << ec.message() << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << save_position();

    snake_game g;

    std::jthread t(input_thread, std::ref(g));

    g.draw();
    while (!g.game_over) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        g.process();
        g.draw();
    }
    t.join();
    return 0;
}
