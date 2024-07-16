#pragma once

#include <ostream>
#include <string>

#include <ansipp/esc.hpp>

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
    static rgb lerp(const rgb& a, const rgb& b, float factor);
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

    std::string value = csi;

    attrs& a(unsigned int param);
    inline unsigned int cb(bool bg, unsigned int base = 38) { return base + (bg ? 10 : 0); }

public:

    /**
     * @brief sets specified foreground/background color
     * @param bg `false` - foreground, `true` - background
     * @param v color to set
     * @param bright bright mode (odd terminal support)
     * @return self
     */
    inline attrs& c(bool bg, color v, bool bright) { return a(cb(bg, 30) + (bright ? 90 : 0) + v); }
    
    /**
     * @brief sets specified foreground/background RGB color
     * @param bg `false` - foreground, `true` - background
     * @param v color to set
     * @return self
     */
    inline attrs& c(bool bg, const rgb& v) { return a(cb(bg)).a(2).a(v.r).a(v.g).a(v.b); }
    
    /**
     * @brief sets specified foreground/background 8-bit color
     * @param bg `false` - foreground, `true` - background
     * @param v color to set
     * @return self
     */
    inline attrs& c(bool bg, unsigned char v) { return a(cb(bg)).a(5).a(v); }
    
    /**
     * @brief sets default foreground/background color
     * @param bg `false` - foreground, `true` - background
     * @return self
     */
    inline attrs& c(bool bg) { return a(cb(bg, 39)); }

    /**
     * @brief sets specified foreground color
     * @param v color to set
     * @param bright bright mode (odd terminal support)
     * @return self
     */
    inline attrs& fg(color v, bool bright = false) { return c(false, v, bright); }
    
    /**
     * @brief sets specified foreground RGB color
     * @param v color to set
     * @return self
     */
    inline attrs& fg(const rgb& v) { return c(false, v); }

    /**
     * @brief sets specified foreground 8-bit color
     * @param v color to set
     * @return self
     */
    inline attrs& fg(unsigned char v) { return c(false, v); }
    
    /**
     * @brief sets default foreground color
     * @return self
     */
    inline attrs& fg() { return c(false); }

    /**
     * @brief sets specified background color
     * @param v color to set
     * @param bright bright mode (odd terminal support)
     * @return self
     */
    inline attrs& bg(color v, bool bright = false) { return c(true, v, bright); }
    
    /**
     * @brief sets specified background RGB color
     * @param v color to set
     * @return self
     */
    inline attrs& bg(const rgb& v) { return c(true, v); }
    
    /**
     * @brief sets specified background 8-bit color
     * @param v color to set
     * @return self
     */
    inline attrs& bg(unsigned char v) { return c(true, v); }
    
    /**
     * @brief sets default background color
     * @return self
     */
    inline attrs& bg() { return c(true); }

    /**
     * @brief enables specified style
     * @param s style to enable
     * @return self
     */
    inline attrs& on(style s) { return a(static_cast<unsigned int>(s)); }
    
    /**
     * @brief disables specified style
     * @param s style to disable
     * @return self
     */
    inline attrs& off(style s) { return a(20 + static_cast<unsigned int>(s == style::BOLD ? style::DIM : s)); }

    /**
     * @brief disables all styles and colors
     * @return self
     */
    inline attrs& off() { return a(0); }

    /**
     * @brief builds ansi sequence
     * @return ansi sequence 
     */
    inline const std::string& str() { return value.append(1, 'm'); }
    
    /**
     * @brief builds ansi sequence
     * @return ansi sequence 
     */
    inline operator const std::string&() { return str(); }

};
inline std::ostream& operator<<(std::ostream& s, attrs&& a) { return s << a.str(); }
inline std::ostream& operator<<(std::ostream& s, attrs& a) { return s << a.str(); }

}