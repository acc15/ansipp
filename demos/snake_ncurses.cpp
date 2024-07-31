#include <iostream>
#include <thread>
#include <algorithm>
#include <vector>
#include <deque>
#include <chrono>
#include <format>
#include <cstdlib>
#include <iostream>
#include <locale>
#include <limits>
#include <unordered_map>
#include <curses.h>

enum color_pair: short {
    BORDER = 1,
    SNAKE,
    APPLE
};

class vec {
public:
    int x;
    int y;

    constexpr vec(): x(), y() {}
    constexpr vec(const vec& b): x(b.x), y(b.y) {}
    explicit constexpr vec(int xy): x(xy), y(xy) {}
    constexpr vec(int x, int y): x(x), y(y) {}
    constexpr vec& operator=(const vec& b)        { x = b.x; y = b.y; return *this; }
    constexpr bool operator==(const vec& b) const { return x == b.x && y == b.y; }
    constexpr vec& operator+=(const vec& b)       { x += b.x; y += b.y; return *this; }
    constexpr vec& operator-=(const vec& b)       { x -= b.x; y -= b.y; return *this; }
    constexpr vec& operator*=(const vec& b)       { x *= b.x; y *= b.y; return *this; }
    constexpr vec& operator/=(const vec& b)       { x /= b.x; y /= b.y; return *this; }
    constexpr vec  operator+(const vec& b) const  { return vec(*this) += b; }
    constexpr vec  operator-(const vec& b) const  { return vec(*this) -= b; }
    constexpr vec  operator*(const vec& b) const  { return vec(*this) *= b; }
    constexpr vec  operator/(const vec& b) const  { return vec(*this) /= b; }
};

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
    std::size_t frame_ns;
    
    std::deque<direction> input_queue;

    snake_game() {
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
        while (true) {
            int v = getch();
            if (v == ERR) break;
            if (v == 'q') return false;        
            if (v == ' ') { if (!undersize) paused = !paused; continue; }
            if (v == KEY_UP) { queue_dir(direction::UP); continue; };
            if (v == KEY_DOWN) { queue_dir(direction::DOWN); continue; }
            if (v == KEY_RIGHT) { queue_dir(direction::RIGHT); continue; }
            if (v == KEY_LEFT) { queue_dir(direction::LEFT); continue; }
        }
        return true;
    }

    void process() {
        getmaxyx(stdscr, terminal_size.y, terminal_size.x);
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
            direction dir = snake[tail];
            snake.erase(tail);
            tail += dir_to_vec(dir);
        }
        process_apples();
    }

    std::string make_border_str(std::string_view content) const {
        std::string str(border_size.x, ' ');
        for (std::size_t i = 0; i < std::min(str.size(), content.size()); ++i) str[i] = content[i];
        return str;
    }

    void draw_border() const {
        attron(COLOR_PAIR(BORDER));

        std::string top_border = make_border_str(" press q to exit, <arrows> to move, <space> to pause/unpause");
        move(0, 0);
        addstr(top_border.c_str());
        for (int y = 0; y < grid_size.y; y++) {
            move(y + 1, 0);
            addch(' ');
            move(y + 1, border_size.x - 1);
            addch(' ');
        }

        std::string bottom_border = make_border_str(std::format(
            " frame={} head={},{} tail={},{} game_over={} apples={} length={}",
            frame_ns, head.x, head.y, tail.x, tail.y, 
            static_cast<unsigned int>(game_over),   
            apples.size(), length));
        move(grid_size.y + 1, 0);
        addstr(bottom_border.c_str());

        attroff(COLOR_PAIR(BORDER));
    }

    void draw_snake() const {
        attron(COLOR_PAIR(SNAKE));
        direction prev_dir = snake.at(tail);
        for (vec cur = tail; cur != head;) {
            direction next_dir = snake.at(cur);
            vec v = dir_to_vec(next_dir);
            move(cur.y + 1, cur.x + 1);
            addstr(get_snake_symbol(prev_dir, next_dir));
            cur += v;
            prev_dir = next_dir;
        }
        move(head.y + 1, head.x + 1); 
        addstr("●");
        attroff(COLOR_PAIR(SNAKE));
    }

    void draw_apples() const {
        attron(COLOR_PAIR(APPLE));
        for (const auto& apple: apples) {
            move(apple.first.y + 1, apple.first.x + 1);
            addstr(apple.second.type == apple_type::SMALL ? "•" : "●");
        }
        attroff(COLOR_PAIR(APPLE));
    }

    void draw_center_text(const std::string& str) {
        move(border_size.y/2, border_size.x/2 - str.size()/2);
        attron(COLOR_PAIR(BORDER));
        addstr(str.c_str());
        attroff(COLOR_PAIR(BORDER));
    }

    void draw() {
        erase();
        if (undersize) {
            addstr(std::format("not enough room to render game, current size {},{} required {},{}", 
                terminal_size.x, terminal_size.y, min_terminal_size.x, min_terminal_size.y).c_str());
        } else {
            draw_border();
            draw_snake();
            draw_apples();
            if (game_over) {
                draw_center_text("GAME IS OVER");
            } else if (paused) {
                draw_center_text("PAUSE");
            }
        }
        refresh();
    }

    void loop() {
        while (true) {
            hr_clock::time_point t1 = hr_clock::now();
            if (!input()) break;
            process();
            draw();
            hr_clock::time_point t2 = hr_clock::now();

            frame_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();

            initializing = false;

            if (game_over) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(80));
        }
    }

};

int main() {
    std::locale::global(std::locale(""));

    WINDOW* window = initscr();
    if (window == nullptr) {
        std::cerr << "can't initscr()" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    if (!has_colors()) {
        endwin();
        std::cerr << "no color support!" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    cbreak();
    noecho();
    keypad(stdscr, true);
    nodelay(stdscr, true);
    start_color();
    init_pair(BORDER, COLOR_BLACK, COLOR_WHITE);
    init_pair(SNAKE, COLOR_GREEN, COLOR_BLACK);
    init_pair(APPLE, COLOR_RED, COLOR_BLACK);
    curs_set(0);
    snake_game().loop();
    endwin();
    return EXIT_SUCCESS;
}