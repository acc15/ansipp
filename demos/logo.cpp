#include <ansipp.hpp>
#include <thread>

// void draw_logo(ansipp::charbuf& out) {

// }

int main() {
    ansipp::init_or_exit();

    ansipp::charbuf out;
    while (true) {


        std::this_thread::sleep_for(std::chrono::milliseconds(40));
    }

    out << "hello, world\n" << ansipp::charbuf::to_stdout;
    return 0;
}