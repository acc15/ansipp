#include <iostream>
#include <iomanip>
#include <ansipp.hpp>

using namespace ansipp;

int main() {

    if (std::error_code ec; init(ec), ec) {
        std::cerr << "can't init: " << ec.message() << std::endl;
        return EXIT_FAILURE;
    }

    std::string buf;
    std::cout << std::hex;    
    while (terminal_read(buf)) {
        std::cout << "BYTES:";
        for (const char ch: buf) {
            std::cout << " 0x" 
                << std::setw(2)
                << std::setfill('0') 
                << std::uppercase 
                << static_cast<unsigned short>(static_cast<unsigned char>(ch));
        }
        std::cout << std::endl;
    }
    return EXIT_SUCCESS;
}