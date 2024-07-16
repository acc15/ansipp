#pragma once

#include <string>

namespace ansipp {

const std::string esc(1, '\x1b');
const std::string csi = esc + '[';
const std::string decset = csi + '?';

}