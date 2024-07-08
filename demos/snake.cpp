#include <iostream>
#include <thread>

#include <ansipp.hpp>
#include <unordered_map>

using namespace ansipp;

struct pt { 
    int x;
    int y; 
};

enum class direction {
    UP, RIGHT, DOWN, LEFT
};

pt dir_to_offset(direction d) {
    switch (d) {
    using enum direction;
    case UP: return { 0, -1 };
    case DOWN: return { 0, 1 };
    case LEFT: return { -1, 0 };
    case RIGHT: return { 1, 0 };
    }
}

enum class object_type {
    EMPTY,
    APPLE,
    SNAKE
};

struct cell {
    object_type type : 2;
    direction dir: 2;
    direction prev_dir: 2;
};

std::string get_symbol(const cell& c) {
    switch (c.type) {
    using enum object_type;
    case EMPTY: return " ";
    case APPLE: return "";
    default: 
        pt po = dir_to_offset(c.prev_dir);
        pt no = dir_to_offset(c.dir);
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
//     { APPLE, "" }
// };

class snake_game {

public:
    static constexpr pt grid_size = { 120, 40 };

    cell grid[grid_size.y][grid_size.x] = {};
    direction dir = direction::RIGHT;
    direction prev_dir = dir;
    pt tail = { 0, 0 };
    pt head = { 2, 0 };
    bool game_over = false;

    snake_game() {
        for (unsigned int i = 0; i < 3; i++) {
            grid[0][i] = cell { object_type::SNAKE, direction::RIGHT, direction::RIGHT };
        }
    }

    void process() {
        if (game_over) { return; }

        pt off = dir_to_offset(dir);
        pt h = { head.x + off.x, head.y + off.y };
        if (h.x < 0 || h.x >= grid_size.x || h.y < 0 || h.y >= grid_size.y) {
            game_over = true;
            return;
        }

        cell& hc = grid[h.y][h.x];
        switch(hc.type) {
        using enum object_type;
        case SNAKE:
            game_over = true;
            return;
            
        case APPLE:
            hc = cell { object_type::SNAKE, dir, prev_dir };
            break;

        case EMPTY:
            hc = cell { object_type::SNAKE, dir, prev_dir };

            cell& tc = grid[tail.y][tail.x];
            tc.type = object_type::EMPTY;
            
            pt toff = dir_to_offset(tc.dir);
            tail = pt { tail.x + toff.x, tail.y + toff.y };
            
            cell& ntc = grid[tail.y][tail.y];
            ntc.prev_dir = ntc.dir;
            break;
        }

        head = h;
        prev_dir = dir;
    }

    void draw() const {
        std::cout << move(0, 0);
        for (unsigned int x = 0; x < grid_size.x + 2; x++) { std::cout << "█"; }
        std::cout << "\n";
        
        for (unsigned int y = 0; y < grid_size.y; y++) {
            std::cout << "█";
            //for (unsigned int x = 0; x < grid_size.x; x++) { std::cout << snake_symbols[grid[y][x]]; }
            std::cout << "█";
            std::cout << "\n";
        }
        
        for (unsigned int x = 0; x < grid_size.x + 2; x++) { std::cout << "█"; }
        std::cout << "\n";
    }

};


int main() {

    if (std::error_code ec; init(ec), ec) {
        std::cerr << "can't init: " << ec.message() << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << hide_cursor() << save_screen() << erase(SCREEN, ALL);

    snake_game g;
    g.draw();
    while (!g.game_over) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        g.process();
        g.draw();
    }
    terminal_getch();

    std::cout << restore_screen();

    // std::cout << "┃┏━━━┓\n";
    // std::cout << "┃┃   ┗━┓\n";
    // std::cout << "┗┛   ⚉━┛\n";
    return 0;
}
