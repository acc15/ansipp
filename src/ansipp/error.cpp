#include <ansipp/error.hpp>

#ifdef _WIN32
#   include <windows.h>
#else
#   include <errno.h>
#endif

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
    case ansipp_error::already_initialized:
        return "ansipp already initialized, call restore() before calling init(ec) again";
    case ansipp_error::at_exit_failure:
        return "unable to set std::atexit handler";
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