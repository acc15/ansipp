#pragma once

namespace ansipp {

struct config {
    
    /**
     * @brief disables terminal input echo
     */
    bool disable_input_echo = true;

    /**
     * @brief enables SIGINT signal handler which restores terminal mode, reset colors/styles and makes cursor visible
     */
    bool enable_signal_restore = true;

    /**
     * @brief enables exit handler which restores terminal mode, reset colors/styles and makes cursor visible
     */
    bool enable_exit_restore = true;

    /**
     * @brief changes windows console encoding to UTF-8
     */
    bool enable_utf8 = true;

};

}