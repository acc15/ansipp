#include <iostream>
#include <iomanip>
#include <thread>
#include <ansipp.hpp>
#include <cstring>

#include <unistd.h>
#include <fcntl.h>

using namespace ansipp;

int main() {

    if (std::error_code ec; init(ec), ec) {
        std::cerr << "can't init: " << ec.message() << std::endl;
        return EXIT_FAILURE;
    }


    int old_flags = fcntl(STDIN_FILENO, F_GETFL);
    fcntl(STDIN_FILENO, F_SETFL, old_flags | O_NONBLOCK);

    char buf[20];

    while (true) {
        ssize_t result = read(STDIN_FILENO, buf, sizeof(buf));
        if (result < 0) {
            std::cout << "ERROR: code = " << errno << " message = " << std::strerror(errno) << std::endl;
        } else {
            std::cout << "BYTES:";
            for (ssize_t i = 0; i < result; i++) {
                std::cout << " 0x" 
                    << std::setw(2)
                    << std::setfill('0') 
                    << std::uppercase 
                    << static_cast<unsigned short>(static_cast<unsigned char>(buf[i]));
            }
            std::cout << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return EXIT_SUCCESS;
}