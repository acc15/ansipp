#pragma once

#include <string>
#include <ansipp/charbuf.hpp>

namespace ansipp {

const std::string esc(1, '\x1b');
const std::string csi = esc + '[';
const std::string decset = csi + '?';

template <typename Esc>
std::string esc_str(const Esc& esc) {
    return (charbuf(16) << esc).str();
}

struct decset_esc {
    unsigned int code;
    char suffix;
};
template <typename Stream>
Stream& operator<<(Stream& s, const decset_esc& e) { return s << decset << e.code << e.suffix; }

class decset_mode {
    unsigned int code;
public:
    constexpr decset_mode(unsigned int code): code(code) {}
    constexpr unsigned int get_code() const { return code; }
    constexpr decset_esc on() const { return decset_esc { code, 'h' }; }
    constexpr decset_esc off() const { return decset_esc { code, 'l' }; }
    
    // doesn't work in Linux Konsole
    // constexpr decset_esc request() const { return esc_prefix() + "$p"; }
    
    // doesn't work in Windows Terminal
    // constexpr decset_esc store() const { return esc_prefix() + 's'; }
    
    // doesn't work in Windows Terminal
    //constexpr decset_esc restore() const { return esc_prefix() + 'r'; }
};


}