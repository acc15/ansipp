#ifdef _WIN32
#   include <windows.h>
#else
#   include <errno.h>
#endif

#include <ansipp/error.hpp>

namespace ansipp {

std::error_code last_error() {
    return std::error_code(
#ifdef _WIN32
        GetLastError()
#else
        errno
#endif
        , std::system_category());
}

const char* ansipp_category_impl::name() const noexcept { 
    return "ansipp"; 
}

std::string ansipp_category_impl::message(int code) const {
    switch (static_cast<ansipp_error>(code)) {
    using enum ansipp_error;
    case not_terminal:
        return "ansipp requires attached terminal";
    case already_initialized:
        return "ansipp already initialized, call restore() before calling init(ec) again";
    case at_exit_failure:
        return "unable to set std::atexit handler";
    case initializing:
        return "ansipp init() currently initializing, never call init() from multiple threads";
    default:
        return "unknown error";
    }
}

const std::error_category& ansipp_category() {
    static ansipp_category_impl instance;
    return instance;
}

std::error_code make_error_code(ansipp_error e) {
    return {static_cast<int>(e), ansipp_category()};
}

}