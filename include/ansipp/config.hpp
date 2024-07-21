#pragma once

#include <string_view>

namespace ansipp {

enum mouse_reporting_mode {
    MOUSE_OFF,
    MOUSE_CLICK,
    MOUSE_CELL,
    MOUSE_ALL
};

enum mouse_reporting_encoding {
    MOUSE_LEGACY,
    MOUSE_UTF8,
    MOUSE_SGR
};

struct config {
    
    /**
     * @brief disables terminal input echo
     */
    bool disable_input_echo = true;

    /**
     * @brief disables generation of signals on corresponding keys (`Ctrl+C` - SIGINT, `Ctrl+\`, `Ctrl+4` - SIGQUIT, etc)
     * @see `man termios`, `ISIG` on linux
     * @see `ENABLE_PROCESSED_INPUT` on Windows
     */
    bool disable_input_signal = false;

    /**
     * @brief enables signal handler which calls restore()
     */
    bool enable_signal_restore = true;

    /**
     * @brief enables `std::atexit` handler which calls restore()
     */
    bool enable_exit_restore = true;

    /**
     * @brief changes Windows console encoding to UTF-8
     */
    bool enable_utf8 = true;

    /**
     * @brief disables stdio sync for `std::cin` and `std::cout`
     */
    bool disable_stdio_sync = true;

    /**
     * @brief automatically reset attrs (colors & styles) on restore()
     */
    bool reset_attrs_on_restore = true;

    /**
     * @brief hides cursor on init(std::error_code&, const config&), shows on restore()
     */
    bool hide_cursor = false;

    /**
     * @brief switches to alternate screen buffer (ncurses like), switches back on restore()
     */
    bool use_alternate_screen_buffer = false;

    /**
     * @brief enables mouse reporting
     */
    mouse_reporting_mode mouse_mode = MOUSE_OFF;

    /**
     * @brief mouse reporting encoding
     */
    mouse_reporting_encoding mouse_encoding = MOUSE_UTF8;

    /**
     * @brief additional custom init string, useful to initialize some additional modes using raw escape sequences
     * @details note that after init(std::error_code, config) call any modifications to this string will be ignored
     */
    std::string_view init_esc = {};

    /**
     * @brief additional custom restore string, useful to restore some additional modes using raw escape sequences
     * @details note that after init(std::error_code, config) call any modifications to this string will be ignored
     */
    std::string_view restore_esc = {};

};

}