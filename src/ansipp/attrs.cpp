#include <ansipp/attrs.hpp>

#include <cmath>

namespace ansipp {

rgb rgb::lerp(const rgb& a, const rgb& b, float factor) {
    return rgb { 
        static_cast<unsigned char>(std::lerp(a.r, b.r, factor)),
        static_cast<unsigned char>(std::lerp(a.g, b.g, factor)),
        static_cast<unsigned char>(std::lerp(a.b, b.b, factor))
    };
}

attrs& attrs::a(unsigned int param) {
    if (!value.empty()) {
        value.append(1, ';');
    }
    value.append(std::to_string(param));
    return *this;
}

}