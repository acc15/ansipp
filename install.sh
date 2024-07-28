#!/bin/bash
set -euxo pipefail
sudo=$(command -v sudo || echo "")
cmake --workflow --preset install
$sudo cmake --build --preset install-debug --target install
$sudo cmake --build --preset install-release --target install
