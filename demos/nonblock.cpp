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
#endif

using namespace ansipp;

// mouse tracking linux https://invisible-island.net/xterm/ctlseqs/ctlseqs.html#h3-Functions-using-CSI-_-ordered-by-the-final-character_s_

int main() {

    if (std::error_code ec; init(ec), ec) {
        std::cerr << "can't init: " << ec.message() << std::endl;
        return EXIT_FAILURE;
    }

#ifndef _WIN32
    int old_flags = fcntl(STDIN_FILENO, F_GETFL);
    fcntl(STDIN_FILENO, F_SETFL, old_flags | O_NONBLOCK);
#endif

    int result;
    char buf[20];
    while (true) {
#ifdef _WIN32
        HANDLE in = GetStdHandle(STD_INPUT_HANDLE);
        if (WaitForSingleObject(in, 0) == WAIT_OBJECT_0) {
            DWORD dwRead;
            ReadFile(in, buf, sizeof(buf), &dwRead, nullptr);
            result = dwRead;
        } else {
            errno = EAGAIN;
            result = -1;
        }
#else
        result = read(STDIN_FILENO, buf, sizeof(buf));
#endif
        if (result < 0) {
            std::cout << "ERROR: " << errno << ": " << strerror(errno) << std::endl;
        } else {
            std::cout << "BYTES:";
            for (int i = 0; i < result; i++) {
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