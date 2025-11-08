@echo off
REM Скрипт для релизной сборки на Windows
REM Требует: CMake, Visual Studio или MinGW, SFML 3.0+

setlocal enabledelayedexpansion

echo ========================================
echo Building GraphVisualizer for Windows
echo ========================================
echo.

REM Определить архитектуру
if "%PROCESSOR_ARCHITECTURE%"=="AMD64" (
    set ARCH=x64
) else if "%PROCESSOR_ARCHITECTURE%"=="ARM64" (
    set ARCH=arm64
) else (
    set ARCH=x86
)

echo Architecture: %ARCH%
echo.

REM Создать директорию для сборки
set BUILD_DIR=build-release-windows-%ARCH%
if exist "%BUILD_DIR%" (
    echo Cleaning old build directory...
    rmdir /s /q "%BUILD_DIR%"
)
mkdir "%BUILD_DIR%"
cd "%BUILD_DIR%"

REM Проверить наличие Visual Studio
where cl >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo Using Visual Studio compiler...
    cmake .. -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 17 2022" -A %ARCH%
    if %ERRORLEVEL% NEQ 0 (
        echo Trying Visual Studio 16 2019...
        cmake .. -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 16 2019" -A %ARCH%
    )
    if %ERRORLEVEL% NEQ 0 (
        echo Trying Visual Studio 15 2017...
        cmake .. -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 15 2017" -A %ARCH%
    )
    if %ERRORLEVEL% NEQ 0 (
        echo Trying default generator...
        cmake .. -DCMAKE_BUILD_TYPE=Release
    )
) else (
    REM Попробовать MinGW
    where g++ >nul 2>&1
    if %ERRORLEVEL% EQU 0 (
        echo Using MinGW compiler...
        cmake .. -DCMAKE_BUILD_TYPE=Release -G "MinGW Makefiles"
    ) else (
        echo ERROR: No C++ compiler found!
        echo Please install Visual Studio or MinGW
        cd ..
        exit /b 1
    )
)

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake configuration failed!
    cd ..
    exit /b 1
)

REM Сборка
echo.
echo Building...
cmake --build . --config Release

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Build failed!
    cd ..
    exit /b 1
)

REM Найти скомпилированный бинарник
set BINARY=
if exist "Release\GraphVisualizer.exe" (
    set BINARY=Release\GraphVisualizer.exe
) else if exist "GraphVisualizer.exe" (
    set BINARY=GraphVisualizer.exe
) else (
    echo ERROR: Binary not found!
    cd ..
    exit /b 1
)

echo.
echo ========================================
echo Build completed successfully!
echo ========================================
echo Binary location: %BUILD_DIR%\%BINARY%
echo.

REM Создать директорию releases
cd ..
if not exist "releases" mkdir "releases"

REM Копировать бинарник
set OUTPUT_NAME=GraphVisualizer-Windows-%ARCH%.exe
copy "%BUILD_DIR%\%BINARY%" "releases\%OUTPUT_NAME%"

if %ERRORLEVEL% EQU 0 (
    echo Binary copied to: releases\%OUTPUT_NAME%
    echo.
    echo ========================================
    echo SUCCESS!
    echo ========================================
) else (
    echo ERROR: Failed to copy binary
    exit /b 1
)
