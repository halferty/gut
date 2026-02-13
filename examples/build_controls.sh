#!/bin/bash
# Build the Controls demo

set -e

echo "Building Gut Controls Demo..."

clang++ -std=c++20 \
    -framework Cocoa \
    -framework Metal \
    -framework QuartzCore \
    -framework CoreVideo \
    -I.. \
    controls_demo.mm \
    -o controls_demo

echo "Build complete! Run with: ./controls_demo"
