#!/bin/sh

BINARY_DIR="${BINARY_DIR:-build/release}"
GENERATOR="${GENERATOR:-Ninja}"

set -euxo pipefail
rm -rf "$BINARY_DIR"
mkdir -p "$BINARY_DIR"
cmake -S . -B "$BINARY_DIR" -G "$GENERATOR" \
    --toolchain "$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
    -DBUILD_TESTING=ON \
    -DBUILD_DEMOS=ON 
cmake --build "$BINARY_DIR"