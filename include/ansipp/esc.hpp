#pragma once

#include <string>
#include <ansipp/charbuf.hpp>

namespace ansipp {

const std::string esc(1, '\x1b');
const std::string csi = esc + '[';
const std::string decset = csi + '?';

template <typename Esc>
std::string esc_str(const Esc& esc) {
    charbuf cb(128);
    cb << esc;
    return cb.str();
}

class decset_mode {
    unsigned int code;
public:
    constexpr decset_mode(unsigned int code): code(code) {}
    constexpr unsigned int get_code() const { return code; }
    constexpr std::string esc_prefix() const { return decset + std::to_string(code); }
    constexpr std::string on() const { return esc_prefix() + 'h'; }
    constexpr std::string off() const { return esc_prefix() + 'l'; }
    
    // doesn't work in Linux Konsole
    constexpr std::string request() const { return esc_prefix() + "$p"; }
    
    // doesn't work in Windows Terminal
    constexpr std::string store() const { return esc_prefix() + 's'; }
    
    // doesn't work in Windows Terminal
    constexpr std::string restore() const { return esc_prefix() + 'r'; }
};

}