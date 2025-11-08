#!/bin/bash
# Скрипт для сборки под все платформы (требует соответствующие инструменты)

set -e

echo "Building for multiple platforms..."
echo "Note: Cross-compilation requires appropriate toolchains and libraries"

# Создать директорию для релизов
mkdir -p releases

# Текущая платформа
PLATFORM=$(uname -s)
ARCH=$(uname -m)

echo "Building for current platform: $PLATFORM ($ARCH)"
./build-release.sh

# Если доступен Docker, можно собрать для Linux
if command -v docker &> /dev/null; then
    echo ""
    echo "Building Linux version using Docker..."
    docker run --rm -v "$(pwd):/workspace" -w /workspace \
        ubuntu:22.04 bash -c "
        apt-get update && \
        apt-get install -y build-essential cmake libsfml-dev && \
        mkdir -p build-release-Linux-x86_64 && \
        cd build-release-Linux-x86_64 && \
        cmake .. -DCMAKE_BUILD_TYPE=Release && \
        cmake --build . --config Release
    " || echo "⚠️  Docker build failed (this is optional)"
fi

echo ""
echo "✅ Build process completed!"
echo "Check the 'releases' directory for binaries"

