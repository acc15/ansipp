#!/bin/bash

sudo=$(command -v sudo || echo "")

BINARY_DIR="${BINARY_DIR:-build/install}"
GENERATOR="${GENERATOR:-Ninja Multi-Config}"

set -euxo pipefail

rm -rf "$BINARY_DIR"
mkdir -p "$BINARY_DIR"
cmake -S . -B "$BINARY_DIR" -G "$GENERATOR" -DBUILD_TESTING=OFF -DBUILD_DEMOS=OFF 
cmake --build "$BINARY_DIR" --config Debug
cmake --build "$BINARY_DIR" --config Release
$sudo cmake --install "$BINARY_DIR" --config Debug
$sudo cmake --install "$BINARY_DIR" --config Release
