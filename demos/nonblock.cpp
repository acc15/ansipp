#include <iostream>
#include <iomanip>
#include <thread>
#include <ansipp.hpp>
#include <cstring>


#ifdef _WIN32
#   include <windows.h>
#else
#   include <unistd.h>
#   include <fcntl.h>
#   include <sys/select.h>
#endif

using namespace ansipp;

// mouse tracking linux https://invisible-island.net/xterm/ctlseqs/ctlseqs.html#h3-Functions-using-CSI-_-ordered-by-the-final-character_s_

int main() {

    if (std::error_code ec; init(ec), ec) {
        std::cerr << "can't init: " << ec.message() << std::endl;
        return EXIT_FAILURE;
    }

    char buf[20];
    while (true) {

        std::string_view rd;
        if (!terminal_read(buf, rd)) { 
            std::cerr << "can't read stdin: " << last_error().message() << std::endl;
            return EXIT_FAILURE;
        }

        std::cout << "BYTES:";
        for (char v: rd) {
            std::cout << " 0x" 
                << std::setw(2)
                << std::setfill('0') 
                << std::uppercase 
                << static_cast<unsigned short>(static_cast<unsigned char>(v));
        }
        std::cout << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return EXIT_SUCCESS;
}