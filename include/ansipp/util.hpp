#pragma once

namespace ansipp {

enum align_type {
    LEFT,
    CENTER,
    RIGHT
};

template <typename T>
T align(align_type type, T container_size, T element_size) {
    switch (type) {
    case CENTER: return (container_size - element_size) / 2;
    case RIGHT: return container_size - element_size;
    default: return 0;
    }
}

}