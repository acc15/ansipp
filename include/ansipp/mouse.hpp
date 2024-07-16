#pragma once

#include <string>
#include <ansipp/esc.hpp>

namespace ansipp {

inline std::string enable_mouse_click()     { return decset + "1000h"; }
inline std::string disable_mouse_click()    { return decset + "1000l"; }

inline std::string enable_mouse_cell()      { return decset + "1002h"; }
inline std::string disable_mouse_cell()     { return decset + "1002l"; }

inline std::string enable_mouse_all()       { return decset + "1003h"; }
inline std::string disable_mouse_all()      { return decset + "1003l"; }

inline std::string enable_mouse_utf8()      { return decset + "1005h"; }
inline std::string disable_mouse_utf8()     { return decset + "1005l"; }

inline std::string enable_mouse_sgr()       { return decset + "1006h"; }
inline std::string disable_mouse_sgr()      { return decset + "1006l"; }

}