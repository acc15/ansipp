#pragma once

#include <cmath>
#include <ostream>
#include <string>
#include <vector>

#include <ansipp/esc.hpp>
#include <ansipp/charbuf.hpp>

namespace ansipp {

enum style {
    BOLD = 1,
    DIM,
    ITALIC,
    UNDERLINE,
    BLINK,
    BLINK_FAST,
    INVERSE,
    HIDDEN,
    STRIKETHROUGH
};

enum color {
    BLACK,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
    WHITE
};

struct rgb {
    
    unsigned char r;
    unsigned char g;
    unsigned char b;

    constexpr rgb(): rgb(0,0,0) {}
    constexpr rgb(unsigned char r, unsigned char g, unsigned char b): r(r), g(g), b(b) {}
    constexpr rgb(int r, int g, int b): 
        r(static_cast<unsigned char>(r)), 
        g(static_cast<unsigned char>(g)), 
        b(static_cast<unsigned char>(b)) {}
    constexpr rgb(int v): rgb((v & 0xff0000) >> 16, (v & 0xff00) >> 8, v & 0xff) {}

    constexpr static rgb lerp(const rgb& a, const rgb& b, float factor) {
        return rgb { 
            static_cast<unsigned char>(std::lerp(a.r, b.r, factor)),
            static_cast<unsigned char>(std::lerp(a.g, b.g, factor)),
            static_cast<unsigned char>(std::lerp(a.b, b.b, factor))
        };
    }
};

/**
 * @brief class for building style and color escape codes
 * 
 * Styles
 *
 * | Enable | Disable | Description                         |
 * |--------|---------|-------------------------------------|
 * | ESC[0m |         | reset all modes (styles and colors) |
 * | ESC[1m | ESC[22m | set bold mode.                      |
 * | ESC[2m | ESC[22m | set dim/faint mode.                 |
 * | ESC[3m | ESC[23m | set italic mode.                    |
 * | ESC[4m | ESC[24m | set underline mode.                 |
 * | ESC[5m | ESC[25m | set blinking mode                   |
 * | ESC[7m | ESC[27m | set inverse/reverse mode            |
 * | ESC[8m | ESC[28m | set hidden/invisible mode           |
 * | ESC[9m | ESC[29m | set strikethrough mode.             |
 * 
 * ## Colors
 *
 * | Color   | FG | BG |
 * |---------|----|----|
 * | Black   | 30 | 40 |
 * | Red     | 31 | 41 |
 * | Green   | 32 | 42 |
 * | Yellow  | 33 | 43 |
 * | Blue    | 34 | 44 |
 * | Magenta | 35 | 45 |
 * | Cyan    | 36 | 46 |
 * | White   | 37 | 47 |
 * | Custom  | 38 | 48 | 5;n (8bit) 2;r;g;b (24bit)
 * | Default | 39 | 49 |
 * 
 * ## Bright colors
 *
 * | Color   | FG | BG  |
 * |---------|----|-----|
 * | Black   | 90 | 100 |
 * | Red     | 91 | 101 |
 * | Green   | 92 | 102 |
 * | Yellow  | 93 | 103 |
 * | Blue    | 94 | 104 |
 * | Magenta | 95 | 105 |
 * | Cyan    | 96 | 106 |
 * | White   | 97 | 107 |
 */
class attrs {

    attrs& a(unsigned int param) { params.push_back(static_cast<unsigned char>(param)); return *this; }
    unsigned char cb(bool bg, unsigned char base = 38) { return base + (bg ? 10 : 0); }

public:
    std::vector<unsigned char> params;

    /**
     * @brief sets specified foreground/background color
     * @param bg `false` - foreground, `true` - background
     * @param v color to set
     * @param bright bright mode (odd terminal support)
     * @return self
     */
    attrs& c(bool bg, color v, bool bright) { return a(cb(bg, 30) + (bright ? 90 : 0) + v); }
    
    /**
     * @brief sets specified foreground/background RGB color
     * @param bg `false` - foreground, `true` - background
     * @param v color to set
     * @return self
     */
    attrs& c(bool bg, const rgb& v) { return a(cb(bg)).a(2).a(v.r).a(v.g).a(v.b); }
    
    /**
     * @brief sets specified foreground/background 8-bit color
     * @param bg `false` - foreground, `true` - background
     * @param v color to set
     * @return self
     */
    attrs& c(bool bg, unsigned char v) { return a(cb(bg)).a(5).a(v); }
    
    /**
     * @brief sets default foreground/background color
     * @param bg `false` - foreground, `true` - background
     * @return self
     */
    attrs& c(bool bg) { return a(cb(bg, 39)); }

    /**
     * @brief sets specified foreground color
     * @param v color to set
     * @param bright bright mode (odd terminal support)
     * @return self
     */
    attrs& fg(color v, bool bright = false) { return c(false, v, bright); }
    
    /**
     * @brief sets specified foreground RGB color
     * @param v color to set
     * @return self
     */
    attrs& fg(const rgb& v) { return c(false, v); }

    /**
     * @brief sets specified foreground 8-bit color
     * @param v color to set
     * @return self
     */
    attrs& fg(unsigned char v) { return c(false, v); }
    
    /**
     * @brief sets default foreground color
     * @return self
     */
    attrs& fg() { return c(false); }

    /**
     * @brief sets specified background color
     * @param v color to set
     * @param bright bright mode (odd terminal support)
     * @return self
     */
    attrs& bg(color v, bool bright = false) { return c(true, v, bright); }
    
    /**
     * @brief sets specified background RGB color
     * @param v color to set
     * @return self
     */
    attrs& bg(const rgb& v) { return c(true, v); }
    
    /**
     * @brief sets specified background 8-bit color
     * @param v color to set
     * @return self
     */
    attrs& bg(unsigned char v) { return c(true, v); }
    
    /**
     * @brief sets default background color
     * @return self
     */
    attrs& bg() { return c(true); }

    /**
     * @brief enables specified style
     * @param s style to enable
     * @return self
     */
    attrs& on(style s) { return a(static_cast<unsigned int>(s)); }
    
    /**
     * @brief disables specified style
     * @param s style to disable
     * @return self
     */
    attrs& off(style s) { return a(20 + static_cast<unsigned int>(s == style::BOLD ? style::DIM : s)); }

    /**
     * @brief disables all styles and colors
     * @return self
     */
    attrs& off() { return a(0); }

    template <typename Stream>
    Stream& out(Stream& s) const {
        s << csi;

        auto it = params.begin();
        const auto end = params.end();
        if (it != end) {
            s << static_cast<unsigned int>(*it);
            ++it;
            for (; it != end; ++it) s << ';' << static_cast<unsigned int>(*it);
        } 
        s << 'm';
        return s;
    }

};
template <typename Stream>
inline Stream& operator<<(Stream& s, const attrs& a) { 
    return a.out(s);
}

}