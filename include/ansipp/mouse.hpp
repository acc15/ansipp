#pragma once

#include <string>

namespace ansipp {

inline std::string enable_mouse_click() { return "\33" "[?1000h"; }
inline std::string disable_mouse_click() { return "\33" "[?1000l"; }

inline std::string enable_mouse_cell() { return "\33" "[?1002h"; }
inline std::string disable_mouse_cell() { return "\33" "[?1002l"; }

inline std::string enable_mouse_all() { return "\33" "[?1003h"; }
inline std::string disable_mouse_all() { return "\33" "[?1003l"; }

inline std::string enable_mouse_utf8() { return "\33" "[?1005h"; }
inline std::string disable_mouse_utf8() { return "\33" "[?1005l"; }

inline std::string enable_mouse_sgr() { return "\33" "[?1006h"; }
inline std::string disable_mouse_sgr() { return "\33" "[?1006l"; }

}