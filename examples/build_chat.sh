#!/bin/bash
set -e
cd "$(dirname "$0")"
clang++ -std=c++20 \
    -framework Cocoa \
    -framework Metal \
    -framework QuartzCore \
    -framework CoreVideo \
    -I.. \
    chat_demo.mm \
    -o chat_demo
echo "Build OK → ./chat_demo"
