#!/bin/bash
# Скрипт для сборки Windows версии через MinGW на Linux/macOS (кросс-компиляция)

set -e

echo "Building Windows version using MinGW cross-compiler..."
echo "Note: This requires MinGW-w64 cross-compiler installed"

# Проверить наличие MinGW
if ! command -v x86_64-w64-mingw32-g++ &> /dev/null; then
    echo "ERROR: MinGW-w64 cross-compiler not found!"
    echo ""
    echo "Installation:"
    echo "  macOS: brew install mingw-w64"
    echo "  Linux: sudo apt-get install mingw-w64"
    exit 1
fi

PLATFORM="Windows"
ARCH="x86_64"

# Создать директорию для сборки
BUILD_DIR="build-release-${PLATFORM}-${ARCH}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Конфигурация CMake для кросс-компиляции
cmake "$SCRIPT_DIR" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_SYSTEM_NAME=Windows \
    -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
    -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ \
    -DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER \
    -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY \
    -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY

# Сборка
cmake --build . --config Release

echo "✅ Windows build completed!"
echo "Binary location: $BUILD_DIR/GraphVisualizer.exe"

# Копировать в releases директорию
cd "$SCRIPT_DIR"
mkdir -p releases
cp "$BUILD_DIR/GraphVisualizer.exe" "releases/GraphVisualizer-${PLATFORM}-${ARCH}.exe"

echo "✅ Binary copied to releases/GraphVisualizer-${PLATFORM}-${ARCH}.exe"
echo ""
echo "Note: You may need to copy SFML DLL files to the same directory as the .exe"

