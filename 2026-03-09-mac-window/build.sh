#!/bin/bash

# Exit on error
set -e

echo "Building Cocoa App..."

# Compile and Link
# -framework Cocoa: Links the necessary AppKit libraries
# -o: Defines the output binary name
clang -dynamiclib -xc logic.c -o liblogic.dylib -framework Foundation
# clang -framework Cocoa -objc main.m -o bitmap_app

echo "Build successful. Running app..."
# ./bitmap_app
