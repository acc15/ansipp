#pragma once

#include <ansipp/config.hpp>
#include "ts_opt.hpp"

#ifdef _WIN32
#   include <windows.h>
#else
#   include <termios.h>
#endif

namespace ansipp {

struct restore_data {
#ifdef _WIN32 // windows
    ts_opt<DWORD> in_modes;
    ts_opt<DWORD> out_modes;
#else // posix
    ts_opt<tcflag_t> lflag;
#endif
    ts_opt<config> init_config;
};

extern restore_data __ansipp_restore;

}