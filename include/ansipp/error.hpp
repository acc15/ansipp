#pragma once

#include <string>
#include <system_error>

namespace ansipp {

/**
 * @brief return last system `error_code` (`GetLastError()` on windows, `errno` on POSIX)
 */
std::error_code last_error();

enum class ansipp_error {
    not_terminal,
    at_exit_failure,
    already_initialized
};

class ansipp_category_impl: public std::error_category {
public:
    const char* name() const noexcept override;
    std::string message(int code) const override;
};

const std::error_category& ansipp_category();
std::error_code make_error_code(ansipp_error e);

}

namespace std {
template <> struct is_error_code_enum<ansipp::ansipp_error> : true_type {};
}