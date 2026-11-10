#!/usr/bin/env bash
set -e

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build"

echo "==> Cleaning previous build..."
rm -rf "$BUILD_DIR"

echo "==> Configuring CMake..."
cmake -B "$BUILD_DIR" -S "$PROJECT_DIR"

echo "==> Building..."
cmake --build "$BUILD_DIR" -j"$(nproc)"

echo "==> Running..."
cd "$PROJECT_DIR"
./OpenGLApp
