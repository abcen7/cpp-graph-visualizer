# Инструкции по сборке релизных версий

Этот документ описывает процесс сборки релизных версий GraphVisualizer для разных платформ.

## Быстрая сборка

### macOS / Linux

```bash
./build-release.sh
```

Бинарник будет создан в директории `releases/GraphVisualizer-<PLATFORM>-<ARCH>`

### Windows

```cmd
build-release-windows.bat
```

Бинарник будет создан в директории `releases\GraphVisualizer-Windows-x64.exe`

## Ручная сборка

### macOS (ARM64 / x86_64)

```bash
mkdir -p build-release
cd build-release
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

### Linux

```bash
# Установить зависимости
sudo apt-get update
sudo apt-get install -y build-essential cmake libsfml-dev

# Сборка
mkdir -p build-release
cd build-release
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

### Windows (Visual Studio)

```cmd
mkdir build-release
cd build-release
cmake .. -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

Или откройте `build-release/GraphVisualizer.sln` в Visual Studio и соберите проект.

## Кросс-компиляция

### Linux на macOS (через Docker)

```bash
docker run --rm -v "$(pwd):/workspace" -w /workspace \
    ubuntu:22.04 bash -c "
    apt-get update && \
    apt-get install -y build-essential cmake libsfml-dev && \
    mkdir -p build-release-Linux && \
    cd build-release-Linux && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . --config Release
"
```

## Оптимизации релизной сборки

Релизная сборка включает следующие оптимизации:

- **GCC/Clang**: `-O3 -DNDEBUG`
- **MSVC**: `/O2 /DNDEBUG`

Размер бинарника оптимизирован, но не минимизирован (не используется `-Os` или `/Os`).

## Структура директорий после сборки

```
releases/
├── GraphVisualizer-Darwin-arm64      # macOS Apple Silicon
├── GraphVisualizer-Darwin-x86_64     # macOS Intel
├── GraphVisualizer-Linux-x86_64      # Linux 64-bit
└── GraphVisualizer-Windows-x64.exe   # Windows 64-bit
```

## Требования для запуска

### macOS
- macOS 10.15 или новее
- SFML 3.0+ (устанавливается через Homebrew: `brew install sfml`)

### Linux
- GLIBC 2.31+ (Ubuntu 20.04+)
- SFML 3.0+ библиотеки

### Windows
- Windows 10 или новее
- Visual C++ Redistributable (обычно уже установлен)
- SFML 3.0+ DLL файлы (должны быть рядом с .exe)

## Проверка сборки

После сборки проверьте бинарник:

```bash
# macOS/Linux
file releases/GraphVisualizer-*
ldd releases/GraphVisualizer-*  # Linux - проверить зависимости

# macOS - проверить архитектуру
lipo -info releases/GraphVisualizer-Darwin-*  # если универсальный бинарник
```

## Устранение проблем

### Ошибка: "SFML not found"
Убедитесь, что SFML 3.0+ установлен:
- macOS: `brew install sfml`
- Linux: `sudo apt-get install libsfml-dev`
- Windows: установите SFML вручную или через vcpkg

### Ошибка: "CMake not found"
Установите CMake 3.15 или новее:
- macOS: `brew install cmake`
- Linux: `sudo apt-get install cmake`
- Windows: скачайте с [cmake.org](https://cmake.org/download/)

