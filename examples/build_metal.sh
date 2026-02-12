#!/bin/bash
# Build the Metal demo

set -e

echo "Building Gut Metal Demo..."

clang++ -std=c++20 \
    -framework Cocoa \
    -framework Metal \
    -framework QuartzCore \
    -framework CoreVideo \
    -I.. \
    metal_demo.mm \
    -o metal_demo

echo "Build complete! Run with: ./metal_demo"
