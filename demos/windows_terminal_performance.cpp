#include <windows.h>
#include <chrono>
#include <thread>

using hr_clock = std::chrono::high_resolution_clock;

int main() {
    std::string str;

    hr_clock::duration d = {};
    while (true) {
        str = std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(d).count()) + '\n';

        HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
        
        DWORD dwWritten;
        auto t1 = hr_clock::now();
        WriteFile(out, str.data(), static_cast<DWORD>(str.size()), &dwWritten, nullptr);
        d = hr_clock::now() - t1;

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    return 0;
}