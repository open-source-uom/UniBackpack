#!/bin/bash
set -e

echo "Building UniBackpack..."

rm -rf build
cmake -B build
cmake --build build

echo "Build complete. Binary at build/UniBackpack"