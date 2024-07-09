#pragma once

#include <sstream>

namespace ansipp {

class terminal_stringbuf: public std::stringbuf {
protected:
    int sync() override;
};

class terminal_stream: public std::ostream {
    terminal_stringbuf buf;
public:
    terminal_stream();
};

}
