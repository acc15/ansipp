#include <iostream>
#include <iomanip>
#include <thread>
#include <cstring>

#ifdef _WIN32
#   include <windows.h>
#else
#   include <unistd.h>
#   include <fcntl.h>
#   include <sys/select.h>
#endif

#include <ansipp.hpp>

using namespace ansipp;

int main() {

    init_or_exit();

    char buf[20];
    while (true) {

        std::string_view rd;
        if (!stdin_read(buf, rd, 0)) { 
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