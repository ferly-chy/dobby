#!/bin/bash

# Dobby host test runner for Termux/Linux development
set -euo pipefail

PROJECT_ROOT=$(pwd)
BUILD_DIR="build/tests"

echo ">>> Preparing host build in $BUILD_DIR..."
rm -rf "$BUILD_DIR"
cmake -S "$PROJECT_ROOT" -B "$BUILD_DIR" -G Ninja \
  -DDOBBY_BUILD_TEST=ON \
  -DDOBBY_BUILD_EXAMPLE=OFF

TARGETS=(test_native test_vtable)
RELOCATION_TARGETS=()
ARCH=$(uname -m)
case "$ARCH" in
  aarch64|arm64)
    RELOCATION_TARGETS+=(test_insn_relo_arm64)
    ;;
  armv7l|arm)
    RELOCATION_TARGETS+=(test_insn_relo_arm)
    ;;
  x86_64)
    RELOCATION_TARGETS+=(test_insn_relo_x64)
    ;;
esac
TARGETS+=("${RELOCATION_TARGETS[@]}")

echo ">>> Building test targets: ${TARGETS[*]}"
cmake --build "$BUILD_DIR" --target "${TARGETS[@]}"

export LD_LIBRARY_PATH="$PROJECT_ROOT/$BUILD_DIR:${LD_LIBRARY_PATH:-}"

echo ""
echo "===================================================="
echo "              RUNNING DOBBY HOST TESTS              "
echo "===================================================="
echo ""

for target in "${TARGETS[@]}"; do
  echo ">>> Running $target"
  "$BUILD_DIR/tests/$target"
done

echo ""
echo "===================================================="
echo ">>> All selected host tests completed."
