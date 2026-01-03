#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/builddir"

# Source the Yocto SDK environment (adjust path as needed)
# source /opt/fsl-imx-xwayland/*/environment-setup-*

# Check if cross-compiler is available
if [ -z "$CC" ]; then
    echo "Error: Yocto SDK environment not sourced."
    echo "Please run: source /path/to/your/sdk/environment-setup-*"
    exit 1
fi

echo "Using CC: $CC"

# Setup meson build with cross-compilation
meson setup "$BUILD_DIR" --cross-file "${SCRIPT_DIR}/yocto_cross.ini" --wipe 2>/dev/null || \
meson setup "$BUILD_DIR" --cross-file "${SCRIPT_DIR}/yocto_cross.ini"

# Build
ninja -C "$BUILD_DIR"

echo "Build complete. Binary at: ${BUILD_DIR}/a53-app-sys-manager"
