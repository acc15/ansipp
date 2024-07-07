#include <signal.h>

#include <ansipp/error.hpp>
#include <ansipp/init.hpp>
#include <ansipp/terminal.hpp>
#include <ansipp/restore.hpp>

#ifdef _WIN32
#   include <windows.h>
#else
#   include <unistd.h>
#   include <termios.h>
#endif

#include "restore.hpp"

namespace ansipp {

void enable_utf8([[maybe_unused]] std::error_code& ec) {
#ifdef _WIN32
    UINT in_cp = GetConsoleCP(); 
    if (in_cp == 0 || (in_cp != CP_UTF8 && !SetConsoleCP(CP_UTF8))) { ec = last_error(); return; }
    
    UINT out_cp = GetConsoleOutputCP(); 
    if (out_cp == 0 || (out_cp != CP_UTF8 && !SetConsoleOutputCP(CP_UTF8))) { ec = last_error(); return; }
#endif
}

void signal_restore(int code) {
    restore();
    std::_Exit(0x80 + code);
}

#ifdef _WIN32
int signal_control_handler(DWORD ctrl_code) {
    if (ctrl_code == CTRL_C_EVENT) {
        signal_restore(2);
    }
    return false;
}
#endif

void enable_signal_restore(std::error_code& ec) {
#ifdef _WIN32 
    if (!SetConsoleCtrlHandler(&signal_control_handler, true)) { ec = last_error(); return; }
#else
    struct sigaction sa;
    sa.sa_handler = &signal_restore;
    if (sigaction(SIGINT, &sa, nullptr) == -1) { ec = last_error(); return; }
#endif
}

void configure_mode(std::error_code& ec, const config& cfg) {
#ifdef _WIN32 // windows

    HANDLE in = GetStdHandle(STD_INPUT_HANDLE);
    if (in == INVALID_HANDLE_VALUE) { ec = last_error(); return; }
    
    DWORD in_modes;
    if (!GetConsoleMode(in, &in_modes)) { ec = last_error(); return; }
    if (!__ansipp_restore.in_modes.store(in_modes)) { ec = ansipp_error::already_initialized; return; }
    if (cfg.disable_input_echo) {
        in_modes &= ~(
            ENABLE_ECHO_INPUT | 
            ENABLE_INSERT_MODE | 
            ENABLE_LINE_INPUT | 
            ENABLE_QUICK_EDIT_MODE |
            ENABLE_WINDOW_INPUT |
            ENABLE_MOUSE_INPUT
        );
    }
    in_modes |= (ENABLE_PROCESSED_INPUT | ENABLE_VIRTUAL_TERMINAL_INPUT);
    if (!SetConsoleMode(in, in_modes)) { ec = last_error(); return; }
    
    HANDLE out;
    if (out = GetStdHandle(STD_OUTPUT_HANDLE); out == INVALID_HANDLE_VALUE) { ec = last_error(); return; }
    
    DWORD out_modes; 
    if (!GetConsoleMode(out, &out_modes)) { ec = last_error(); return; }

    if (!__ansipp_restore.out_modes.store(out_modes)) { ec = ansipp_error::already_initialized; return; }
    out_modes |= (ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    if (!SetConsoleMode(out, out_modes)) { ec = last_error(); return; }

#else // posix
    if (cfg.disable_input_echo) {
        termios p; 
        if (tcgetattr(STDOUT_FILENO, &p) == -1) { ec = last_error(); return; }
        if (!__ansipp_restore.lflag.store(p.c_lflag)) { ec = ansipp_error::already_initialized; return; }
        p.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL); 
        if (tcsetattr(STDOUT_FILENO, TCSANOW, &p) == -1) { ec = last_error(); return; }
    }
#endif
}

void init(std::error_code& ec, const config& cfg) {
    if (!is_terminal()) { ec = ansipp_error::not_terminal; return; }
    if (cfg.enable_exit_restore && std::atexit(&restore) != 0) { ec = ansipp_error::at_exit_failure; return; }
    if (cfg.enable_signal_restore && (enable_signal_restore(ec), ec)) { return; }
    if (cfg.enable_utf8 && (enable_utf8(ec), ec)) { return; }
    if (configure_mode(ec, cfg), ec) { return; }
}


}