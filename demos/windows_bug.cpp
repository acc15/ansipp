#include <windows.h>
#include <conio.h>
#include <iostream>
#include <iomanip>
#include <thread>


void peek_console_input(HANDLE in) {
    INPUT_RECORD records[1024];
    DWORD dwRead;
    PeekConsoleInput(in, records, 1024, &dwRead);

    std::cout << dwRead << " records peeked" << std::endl;
    for (unsigned int i = 0; i < dwRead; i++) {
        switch (records[i].EventType) {
            case FOCUS_EVENT: std::cout << "FOCUS_EVENT"; break;
            case KEY_EVENT: std::cout << "KEY_EVENT"; break;
            case MENU_EVENT: std::cout << "MENU_EVENT"; break;
            case MOUSE_EVENT: std::cout << "MOUSE_EVENT"; break;
            case WINDOW_BUFFER_SIZE_EVENT: std::cout << "WINDOW_BUFFER_SIZE_EVENT"; break;
        }
        std::cout << std::endl;
    }
}

void read_console_bytes(HANDLE in) {
    unsigned char buf[1024];
    DWORD dwBytes;
    ReadConsole(in, buf, sizeof(buf), &dwBytes, NULL);
    std::cout << dwBytes << " bytes read" << std::endl;

    for (unsigned int i = 0; i < dwBytes; i++) {
        std::cout << " 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(buf[i]);
    }
    std::cout << std::endl;
}

int main() {

    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);


    HANDLE in = GetStdHandle(STD_INPUT_HANDLE);
    SetConsoleMode(in, ENABLE_PROCESSED_INPUT | ENABLE_VIRTUAL_TERMINAL_INPUT); // 
    
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleMode(out, ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

    CONSOLE_SCREEN_BUFFER_INFO b;
    if (!GetConsoleScreenBufferInfo(out, &b)) {
        std::cerr << "GetConsoleScreenBufferInfo fail" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::cout << "GetConsoleScreenBufferInfo " 
        << "left=" << b.srWindow.Left 
        << ",top=" << b.srWindow.Top 
        << ",right=" << b.srWindow.Right
        << ",bottom=" << b.srWindow.Bottom
        << std::endl;

    std::cout << "try to resize terminal window in 5 second timeout..." << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::cout << "after sleep" << std::endl;

    DWORD dwResult = WaitForSingleObject(in, 0);
    std::cout << "WaitForSingleObject done with " << dwResult << std::endl;

    if (dwResult == WAIT_OBJECT_0) {
        peek_console_input(in);
        read_console_bytes(in);
        peek_console_input(in);
    }

    std::cout << std::string((b.srWindow.Right - b.srWindow.Left) + 1, 'a') << std::endl;

    return EXIT_SUCCESS;
}