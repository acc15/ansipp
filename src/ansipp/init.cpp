#include <iostream>

#ifdef _WIN32
#   include <windows.h>
#else
#   include <unistd.h>
#   include <termios.h>
#   include <signal.h>
#endif

#include <ansipp/error.hpp>
#include <ansipp/init.hpp>
#include <ansipp/terminal.hpp>
#include <ansipp/restore.hpp>
#include <ansipp/io.hpp>
#include <ansipp/cursor.hpp>
#include <ansipp/attrs.hpp>

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

void signal_restore(int signal) {
    restore(); // restore will also restore old sigactions
#ifndef _WIN32
    raise(signal); // original signal handler was restored - just re-raising signal
#endif
}

#ifdef _WIN32
int signal_ctrl_handler(DWORD ctrl_code) {
    if (ctrl_code == CTRL_C_EVENT) {
        signal_restore(2);
    }
    return false;
}
#else
void init_signal_handler(std::error_code& ec, int sig, ts_opt<struct sigaction>& restore) {
    struct sigaction sa, sa_old;
    sa.sa_handler = &signal_restore;
    if (restore.is_set()) { ec = ansipp_error::already_initialized; return; }
    if (sigaction(sig, &sa, &sa_old) == -1) { ec = last_error(); return; }
    if (!restore.store(sa_old)) { ec = ansipp_error::already_initialized; return; }
}
#endif

void enable_signal_restore(std::error_code& ec) {
#ifdef _WIN32 
    if (!__ansipp_restore.ctrl_handler.store(&signal_ctrl_handler)) { ec = ansipp_error::already_initialized; return; }
    if (!SetConsoleCtrlHandler(&signal_ctrl_handler, true)) { ec = last_error(); return; }
#else
    if (init_signal_handler(ec, SIGINT, __ansipp_restore.sigint), ec) { return; }
    if (init_signal_handler(ec, SIGTERM, __ansipp_restore.sigterm), ec) { return; }
    if (init_signal_handler(ec, SIGQUIT, __ansipp_restore.sigquit), ec) { return; }
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
        in_modes = 0;
    }
    if (!cfg.disable_input_signal) {
        in_modes |= ENABLE_PROCESSED_INPUT;
    }
    in_modes |= ENABLE_VIRTUAL_TERMINAL_INPUT;
    if (!SetConsoleMode(in, in_modes)) { ec = last_error(); return; }
    
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    if (out == INVALID_HANDLE_VALUE) { ec = last_error(); return; }
    
    DWORD out_modes; 
    if (!GetConsoleMode(out, &out_modes)) { ec = last_error(); return; }

    if (!__ansipp_restore.out_modes.store(out_modes)) { ec = ansipp_error::already_initialized; return; }
    out_modes |= (ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    if (!SetConsoleMode(out, out_modes)) { ec = last_error(); return; }

#else // posix
    if (cfg.disable_input_echo || cfg.disable_input_signal) {
        termios p; 
        if (tcgetattr(STDOUT_FILENO, &p) == -1) { ec = last_error(); return; }
        if (!__ansipp_restore.lflag.store(p.c_lflag)) { ec = ansipp_error::already_initialized; return; }
        if (cfg.disable_input_echo) p.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL);
        if (cfg.disable_input_signal) p.c_lflag &= ~ISIG;
        if (tcsetattr(STDOUT_FILENO, TCSAFLUSH, &p) == -1) { ec = last_error(); return; }
    }
#endif
}

void disable_stdio_sync() {
    std::cin.sync_with_stdio(false);
    std::cout.sync_with_stdio(false);
}

void atexit_restore(std::error_code& ec) {
    if (std::atexit(&restore) != 0) ec = ansipp_error::at_exit_failure;
}

void configure_escapes(const config& cfg, std::error_code& ec) {
    std::string init_esc, restore_esc;
    if (cfg.reset_attrs_on_restore) {
        restore_esc += attrs().str();
    }
    if (cfg.hide_cursor) {
        init_esc    += hide_cursor();
        restore_esc += show_cursor();
    }
    if (cfg.use_alternate_screen_buffer) { 
        init_esc    += enable_alternate_buffer(); 
        restore_esc += disable_alternate_buffer(); 
    }
    if (cfg.enable_mouse_reporting) {
        init_esc    += enable_mouse_reporting();
        restore_esc += disable_mouse_reporting();
    }
    init_esc += cfg.init_esc;
    if (terminal_write(init_esc) < 0) { ec = last_error(); return; }

    restore_esc += cfg.restore_esc;
    if (!__ansipp_restore.escapes.store(std::move(restore_esc))) { ec = ansipp_error::already_initialized; return; }
}

void init_non_restorable(std::error_code& ec, const config& cfg) {
    if (!is_terminal()) { ec = ansipp_error::not_terminal; return; }
    if (cfg.enable_utf8 && (enable_utf8(ec), ec)) { return; }
    if (cfg.disable_stdio_sync) { disable_stdio_sync(); }
}

void init_restorable(std::error_code& ec, const config& cfg) {
    if (configure_mode(ec, cfg), ec) return;
    if (configure_escapes(cfg, ec), ec) return;
    if (cfg.enable_exit_restore && (atexit_restore(ec), ec)) return;
    if (cfg.enable_signal_restore && (enable_signal_restore(ec), ec)) return;
}

struct init_guard {
    bool other_thread_init;
    init_guard(): other_thread_init(__ansipp_restore.initializing.exchange(true)) {}
    ~init_guard() { if (!other_thread_init) __ansipp_restore.initializing.store(false); }
};

void init(std::error_code& ec, const config& cfg) {
    init_guard g; 
    if (g.other_thread_init) { ec = ansipp_error::initializing; return; }
    if (init_non_restorable(ec, cfg), ec) return; // nothing to restore if any of these will fail
    if (init_restorable(ec, cfg), ec) restore();
}

}