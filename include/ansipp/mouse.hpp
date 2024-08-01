#pragma once

#include <ansipp/esc.hpp>

namespace ansipp {

constexpr decset_mode mouse_click = 1000;
constexpr decset_mode mouse_cell  = 1002;
constexpr decset_mode mouse_all   = 1003;
constexpr decset_mode mouse_utf8  = 1005;
constexpr decset_mode mouse_sgr   = 1006;

enum mouse_mode {
    MOUSE_OFF,
    MOUSE_CLICK,
    MOUSE_CELL,
    MOUSE_ALL
};
constexpr const decset_mode* get_mouse_mode_decset(mouse_mode mode) {
    switch (mode) {
        case MOUSE_CLICK: return &mouse_click;
        case MOUSE_CELL: return &mouse_cell;
        case MOUSE_ALL: return &mouse_all;
        default: return nullptr;
    }
}

enum mouse_encoding {
    MOUSE_LEGACY,
    MOUSE_UTF8,
    MOUSE_SGR
};
constexpr const decset_mode* get_mouse_encoding_decset(mouse_encoding enc) {
    switch (enc) {
        case MOUSE_UTF8: return &mouse_utf8;
        case MOUSE_SGR: return &mouse_sgr;
        default: return nullptr;
    }
}

}