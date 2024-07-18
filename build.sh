#!/bin/sh

BINARY_DIR="${BINARY_DIR:-build/release}"
GENERATOR="${GENERATOR:-Ninja}"

set -euxo pipefail
if [[ ! -d $BINARY_DIR ]]; then
    rm -rf "$BINARY_DIR"
    mkdir -p "$BINARY_DIR"
    cmake -S . -B "$BINARY_DIR" -G "$GENERATOR" \
        --toolchain "$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_TESTING=ON \
        -DBUILD_DEMOS=ON 
fi
cmake --build "$BINARY_DIR"