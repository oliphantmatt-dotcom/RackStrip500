#!/usr/bin/env bash
set -e

echo "======================================================================"
echo "          RackStrip500 JUCE VST3 Plugin Build Script                  "
echo "======================================================================"

BUILD_DIR="build"
BUILD_TYPE="Release"

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

NPROC=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo "--> Configuring CMake project (Build Type: ${BUILD_TYPE})..."
cmake .. -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"

echo "--> Building target RackStrip500 with ${NPROC} parallel jobs..."
cmake --build . --config "${BUILD_TYPE}" -j"${NPROC}"

echo "======================================================================"
echo " Build completed successfully!"
echo " Binaries located in: $(pwd)"
echo "======================================================================"
