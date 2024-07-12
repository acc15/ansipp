#include <iostream>
#include <iomanip>
#include <ansipp.hpp>

using namespace ansipp;

int main() {

    if (std::error_code ec; init(ec), ec) {
        std::cerr << "can't init: " << ec.message() << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << std::hex;    

    std::string buf(512, '\0');
    while (true) {
        
        std::string_view rd;
        if (!terminal_read(buf, rd)) { 
            std::cerr << "can't read stdin: " << last_error().message() << std::endl;
            return EXIT_FAILURE;
        }

        std::cout << "BYTES:";
        for (const char ch: rd) {
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