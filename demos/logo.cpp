#include <ansipp.hpp>
#include <string_view>

using namespace ansipp;

const std::string_view logo = R"(
                 _             
                (_)            
  __ _ _ __  ___ _ _ __  _ __  
 / _` | '_ \/ __| | '_ \| '_ \ 
| (_| | | | \__ \ | |_) | |_) |
 \__,_|_| |_|___/_| .__/| .__/ 
                  | |   | |    
                  |_|   |_|    
)";

void draw_logo(charbuf& out) {
    out << logo.substr(1, logo.size() - 1); 
}

void draw_terminal(charbuf& out) {
    out << "╭────╮\n"
           "│" << attrs().fg(GREEN).bg(rgb{ 50, 50, 50 }) << " >_ " << attrs() << "│\n"
           "╰────╯\n";
}

int main() {
    init_or_exit();

    charbuf out(4096);
    draw_logo(out);
    terminal_write(out.flush());
    return EXIT_SUCCESS;
}