#pragma once

#include <ostream>

namespace ansipp {

class vec {
public:
    int x;
    int y;

    constexpr vec(): x(), y() {}
    constexpr vec(const vec& b): x(b.x), y(b.y) {}
    constexpr vec(int xy): x(xy), y(xy) {}
    constexpr vec(int x, int y): x(x), y(y) {}
    constexpr vec& operator=(const vec& b)        { x = b.x; y = b.y; return *this; }
    constexpr bool operator==(const vec& b) const { return x == b.x && y == b.y; }
    constexpr vec& operator+=(const vec& b)       { x += b.x; y += b.y; return *this; }
    constexpr vec& operator-=(const vec& b)       { x -= b.x; y -= b.y; return *this; }
    constexpr vec& operator*=(const vec& b)       { x *= b.x; y *= b.y; return *this; }
    constexpr vec& operator/=(const vec& b)       { x /= b.x; y /= b.y; return *this; }
    constexpr vec  operator+(const vec& b) const  { return vec(*this) += b; }
    constexpr vec  operator-(const vec& b) const  { return vec(*this) -= b; }
    constexpr vec  operator*(const vec& b) const  { return vec(*this) *= b; }
    constexpr vec  operator/(const vec& b) const  { return vec(*this) /= b; }
};
inline std::ostream& operator<<(std::ostream& o, const vec& p) {
    return o << p.x << "," << p.y;
}

}