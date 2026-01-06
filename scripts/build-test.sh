#!/bin/bash
# Local build test script - Simulates GitHub Actions workflow

set -e

echo "======================================"
echo "SinricPro Pico SDK - Build Test"
echo "======================================"
echo ""

# Check for pico-sdk
if [ -z "$PICO_SDK_PATH" ]; then
    echo "ERROR: PICO_SDK_PATH environment variable not set"
    echo "Please set it to your pico-sdk installation directory:"
    echo "  export PICO_SDK_PATH=/path/to/pico-sdk"
    exit 1
fi

if [ ! -d "$PICO_SDK_PATH" ]; then
    echo "ERROR: PICO_SDK_PATH directory does not exist: $PICO_SDK_PATH"
    exit 1
fi

echo "Using pico-sdk: $PICO_SDK_PATH"
echo ""

# Check for cJSON submodule
echo "Checking cJSON submodule..."
if [ ! -d "lib/cJSON/.git" ]; then
    echo "Initializing cJSON submodule..."
    git submodule add https://github.com/DaveGamble/cJSON.git lib/cJSON 2>/dev/null || true
    git submodule update --init --recursive
else
    echo "cJSON submodule already initialized"
fi
echo ""

# Clean previous build
if [ -d "build" ]; then
    echo "Cleaning previous build..."
    rm -rf build
fi

# Configure
echo "Configuring CMake..."
cmake -B build -DPICO_BOARD=pico_w

echo ""
echo "======================================"
echo "Building all examples..."
echo "======================================"
echo ""

# Build
cmake --build build -j$(nproc)

echo ""
echo "======================================"
echo "Build Summary"
echo "======================================"
echo ""

# List built artifacts
echo "Built examples (.uf2 files):"
echo ""
find build/examples -name "*.uf2" | while read -r file; do
    size=$(stat -f%z "$file" 2>/dev/null || stat -c%s "$file")
    size_kb=$((size / 1024))
    name=$(basename "$file" .uf2)
    echo "  ✅ $name (${size_kb}KB)"
done

echo ""
echo "======================================"
echo "Build test completed successfully!"
echo "======================================"
echo ""
echo "To flash an example to your Pico W:"
echo "  1. Hold BOOTSEL button"
echo "  2. Connect Pico W via USB"
echo "  3. Copy .uf2 file to RPI-RP2 drive"
echo ""
