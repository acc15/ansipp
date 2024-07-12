#include <iostream>
#include <iomanip>
#include <thread>
#include <ansipp.hpp>

using namespace ansipp;


int main() {

    if (std::error_code ec; init(ec), ec) {
        std::cerr << "can't init: " << ec.message() << std::endl;
        return EXIT_FAILURE;
    }

    for (std::size_t i = 0; i < 5; i++) std::cout << '\n';
    std::cout << std::flush;
    return 0;
}

