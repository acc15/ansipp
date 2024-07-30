#include <thread>
#include <ansipp.hpp>

using namespace ansipp;

int main() {

    init_or_exit();

    charbuf out(1024);

    char buf[20];
    while (true) {

        std::string_view rd;
        if (!stdin_read(buf, rd, 0)) { 
            out << "can't read stdin: " << last_error().message() << '\n' << charbuf::to_stderr;
            return EXIT_FAILURE;
        }

        out << "BYTES:";
        for (char v: rd) {
            out << " 0x" << integral_format(v, 16, true, 2);
        }
        out << '\n' << charbuf::to_stdout;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return EXIT_SUCCESS;
}