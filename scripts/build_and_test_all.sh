#!/bin/bash
set -euo pipefail

PROJECT_ROOT=$(pwd)
BUILD_DIR="build/cmake-build-all-features"
ARCH="${ANDROID_ABI:-arm64-v8a}"
NDK="${ANDROID_NDK_HOME:-}"

if [ -z "$NDK" ]; then
    echo "Error: ANDROID_NDK_HOME is not set."
    exit 1
fi

echo "[*] Cleaning previous Android build..."
rm -rf "$BUILD_DIR"

echo "[*] Configuring Android CMake build with all features enabled..."
cmake -S "$PROJECT_ROOT" -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Release \
    -DDOBBY_BUILD_TEST=ON \
    -DDOBBY_BUILD_EXAMPLE=ON \
    -DPlugin.SymbolResolver=ON \
    -DPlugin.ImportTableReplace=ON \
    -DPlugin.Android.BionicLinkerUtil=ON \
    -DCMAKE_TOOLCHAIN_FILE="$NDK/build/cmake/android.toolchain.cmake" \
    -DANDROID_ABI="$ARCH" \
    -DANDROID_PLATFORM=android-21 \
    -DANDROID_STL=c++_static \
    -G Ninja

echo "[*] Building all Android targets..."
cmake --build "$BUILD_DIR"

echo ""
echo "[*] Android build completed for ABI: $ARCH"
echo "[*] Note: Android test binaries were built successfully, but are not executed by this script."
echo "[*] Use scripts/test_runner.sh for host/Termux execution, or deploy the Android binaries to a compatible runtime before running them."
