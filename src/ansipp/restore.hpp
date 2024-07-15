#pragma once

#include <ansipp/config.hpp>
#include "ts_opt.hpp"

#ifdef _WIN32
#   include <windows.h>
#else
#   include <termios.h>
#   include <signal.h>
#endif

namespace ansipp {

struct restore_data {
#ifdef _WIN32 // windows
    ts_opt<DWORD> in_modes;
    ts_opt<DWORD> out_modes;
    ts_opt<PHANDLER_ROUTINE> ctrl_handler;
#else // posix
    ts_opt<tcflag_t> lflag;
    ts_opt<struct sigaction> sigint;
    ts_opt<struct sigaction> sigterm;
#endif
    ts_opt<std::string> escapes;
};

extern restore_data __ansipp_restore;

}