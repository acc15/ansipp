#pragma once

namespace ansipp {

struct config {
    
    /**
     * @brief disables terminal input echo
     */
    bool disable_input_echo = true;

    /**
     * @brief enables signal handler which call restore()
     */
    bool enable_signal_restore = true;

    /**
     * @brief enables exit handler which calls restore()
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
     * @brief hides cursor on init(std::error_code&, const config&), show on restore()
     */
    bool hide_cursor = false;

    /**
     * @brief switches to alternate screen buffer (ncurses like), switches back on restore()
     */
    bool use_alternate_screen_buffer = false;

};

}