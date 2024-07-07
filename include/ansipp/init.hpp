#pragma once

#include <system_error>
#include "config.hpp"

namespace ansipp {

void init(std::error_code& ec, const config &cfg = {});

}