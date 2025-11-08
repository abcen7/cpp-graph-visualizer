#!/bin/bash
# Скрипт для релизной сборки на macOS/Linux

set -e

PLATFORM=$(uname -s)
ARCH=$(uname -m)

echo "Building release version for $PLATFORM ($ARCH)..."

# Получить абсолютный путь к директории скрипта
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Создать директорию для релизной сборки
BUILD_DIR="build-release-${PLATFORM}-${ARCH}"
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Конфигурация CMake для релизной сборки
cmake "$SCRIPT_DIR" -DCMAKE_BUILD_TYPE=Release

# Сборка
cmake --build . --config Release

echo "✅ Release build completed!"
echo "Binary location: $BUILD_DIR/GraphVisualizer"

# Копировать в releases директорию
cd ..
mkdir -p releases
cp "$BUILD_DIR/GraphVisualizer" "releases/GraphVisualizer-${PLATFORM}-${ARCH}"

echo "✅ Binary copied to releases/GraphVisualizer-${PLATFORM}-${ARCH}"

