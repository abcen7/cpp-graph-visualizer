# Сборка GraphVisualizer для Windows

## Быстрый старт

### Вариант 1: Visual Studio (рекомендуется)

1. **Установите зависимости:**
   - [Visual Studio 2022](https://visualstudio.microsoft.com/downloads/) (Community Edition достаточно)
     - При установке выберите "Desktop development with C++"
   - [CMake](https://cmake.org/download/) 3.15 или новее
   - [SFML 3.0+](https://www.sfml-dev.org/download.php)

2. **Настройте SFML:**
   - Скачайте SFML 3.0 для Windows
   - Распакуйте в `C:\SFML` или другое место
   - Установите переменную окружения `SFML_ROOT` или укажите путь в CMake

3. **Соберите проект:**
   ```cmd
   build-release-windows.bat
   ```

### Вариант 2: MinGW-w64

1. **Установите зависимости:**
   - [MSYS2](https://www.msys2.org/) или [MinGW-w64](https://www.mingw-w64.org/)
   - CMake
   - SFML 3.0+ (скомпилированный для MinGW)

2. **Соберите проект:**
   ```cmd
   build-release-windows.bat
   ```

## Ручная сборка

### Visual Studio

```cmd
mkdir build-release-windows
cd build-release-windows
cmake .. -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

Или откройте `build-release-windows/GraphVisualizer.sln` в Visual Studio и соберите проект.

### MinGW

```cmd
mkdir build-release-windows
cd build-release-windows
cmake .. -DCMAKE_BUILD_TYPE=Release -G "MinGW Makefiles"
cmake --build . --config Release
```

## Установка SFML для Windows

### Способ 1: Через vcpkg (рекомендуется)

```cmd
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg install sfml:x64-windows
```

Затем укажите toolchain при конфигурации CMake:
```cmd
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake
```

### Способ 2: Ручная установка

1. Скачайте SFML 3.0 для Windows с [официального сайта](https://www.sfml-dev.org/download.php)
2. Распакуйте в `C:\SFML` или другое место
3. Установите переменную окружения:
   ```cmd
   set SFML_ROOT=C:\SFML
   ```

Или укажите путь в CMake:
```cmd
cmake .. -DSFML_ROOT=C:\SFML -DCMAKE_BUILD_TYPE=Release
```

## Архитектуры

Скрипт автоматически определяет архитектуру:
- `x64` - 64-bit (по умолчанию)
- `arm64` - ARM64 (Windows on ARM)
- `x86` - 32-bit

Для явного указания архитектуры в Visual Studio:
```cmd
cmake .. -A x64    # или arm64, Win32
```

## Результат

После успешной сборки бинарник будет в:
- `releases/GraphVisualizer-Windows-x64.exe` (или arm64/x86)

## Требования для запуска

- Windows 10 или новее
- Visual C++ Redistributable (обычно уже установлен)
- SFML DLL файлы (если не статически слинкованы):
  - `sfml-graphics-3.dll`
  - `sfml-window-3.dll`
  - `sfml-system-3.dll`

Эти DLL должны быть в той же директории, что и .exe файл, или в PATH.

## Устранение проблем

### Ошибка: "SFML not found"

1. Убедитесь, что SFML установлен
2. Установите переменную окружения `SFML_ROOT`
3. Или укажите путь в CMake: `cmake .. -DSFML_ROOT=C:\SFML`

### Ошибка: "No C++ compiler found"

- Установите Visual Studio с компонентом "Desktop development with C++"
- Или установите MinGW-w64 и добавьте в PATH

### Ошибка: "CMake not found"

- Скачайте CMake с [cmake.org](https://cmake.org/download/)
- Добавьте CMake в PATH
- Или используйте Visual Studio, которая включает CMake

### Ошибка при запуске: "DLL not found"

- Скопируйте SFML DLL файлы в директорию с .exe
- Или добавьте директорию с SFML DLL в PATH

## Статическая линковка (опционально)

Для создания standalone .exe без внешних DLL, нужно скомпилировать SFML статически или использовать статические библиотеки.

