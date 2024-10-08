#pragma once

#include <atomic>
#ifdef _WIN32
#   include <windows.h>
#else
#   include <termios.h>
#   include <signal.h>
#endif

#include <ansipp/config.hpp>
#include <ansipp/charbuf.hpp>
#include "ts_opt.hpp"

namespace ansipp {

struct restore_data {
    std::atomic_bool initializing;
#ifdef _WIN32 // windows
    ts_opt<DWORD> in_modes;
    ts_opt<DWORD> out_modes;
    ts_opt<PHANDLER_ROUTINE> ctrl_handler;
#else // posix
    ts_opt<tcflag_t> lflag;
    ts_opt<struct sigaction> sigint;
    ts_opt<struct sigaction> sigterm;
    ts_opt<struct sigaction> sigquit;
#endif
    ts_opt<charbuf> escapes;
};

extern restore_data __ansipp_restore;

}