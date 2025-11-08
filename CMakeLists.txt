cmake_minimum_required(VERSION 3.15)
project(GraphVisualizer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SFML 3
find_package(SFML 3 REQUIRED COMPONENTS Window Graphics)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)
# Добавить SFML include директорию заранее (для совместимости с SFML 2.6)
if(APPLE AND EXISTS /opt/homebrew/include/SFML)
    include_directories(/opt/homebrew/include)
endif()

# Source files
set(CORE_SOURCES
    src/core/graph.cpp
    src/core/algorithms.cpp
    src/core/parallel.cpp
)

set(IO_SOURCES
    src/io/loader.cpp
)

set(VISUALIZATION_SOURCES
    src/visualization/layout.cpp
    src/visualization/renderer.cpp
)

set(SOURCES
    ${CORE_SOURCES}
    ${IO_SOURCES}
    ${VISUALIZATION_SOURCES}
    src/main.cpp
)

# Executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries - SFML 3 использует цели CMake
if(TARGET SFML::Window AND TARGET SFML::Graphics)
    target_link_libraries(${PROJECT_NAME} PRIVATE 
        SFML::Graphics
        SFML::Window
    )
elseif(APPLE)
    # Fallback для macOS
    link_directories(/opt/homebrew/lib)
    target_link_libraries(${PROJECT_NAME} PRIVATE 
        sfml-graphics
        sfml-window
        sfml-system
    )
elseif(TARGET SFML::System AND TARGET SFML::Window AND TARGET SFML::Graphics)
    # Для других платформ используем цели
    target_link_libraries(${PROJECT_NAME} 
        PRIVATE 
        SFML::Graphics
        SFML::Window
        SFML::System
    )
    # Получить include директории из цели Graphics
    get_target_property(SFML_INCLUDE_DIRS SFML::Graphics INTERFACE_INCLUDE_DIRECTORIES)
    if(SFML_INCLUDE_DIRS AND NOT "${SFML_INCLUDE_DIRS}" STREQUAL "SFML_INCLUDE_DIRS-NOTFOUND")
        target_include_directories(${PROJECT_NAME} PRIVATE ${SFML_INCLUDE_DIRS})
    elseif(SFML_INCLUDE_DIR AND NOT "${SFML_INCLUDE_DIR}" STREQUAL "SFML_INCLUDE_DIR-NOTFOUND")
        target_include_directories(${PROJECT_NAME} PRIVATE ${SFML_INCLUDE_DIR})
    else()
        # Fallback: использовать стандартные пути (всегда добавляем для надежности)
        if(EXISTS /opt/homebrew/include/SFML)
            target_include_directories(${PROJECT_NAME} PRIVATE /opt/homebrew/include)
        endif()
        if(EXISTS /usr/local/include/SFML)
            target_include_directories(${PROJECT_NAME} PRIVATE /usr/local/include)
        endif()
    endif()
    # Всегда добавляем /opt/homebrew/include если это macOS и Homebrew установлен
    if(APPLE AND EXISTS /opt/homebrew/include/SFML)
        target_include_directories(${PROJECT_NAME} PRIVATE /opt/homebrew/include)
    endif()
else()
    # Альтернативный способ для старых версий или если цели не экспортированы
    target_include_directories(${PROJECT_NAME} PRIVATE ${SFML_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} 
        PRIVATE 
        ${SFML_LIBRARIES}
    )
endif()

# Compiler-specific options
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
    # Release optimizations for MSVC
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /DNDEBUG")
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
    # Release optimizations for GCC/Clang
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
endif()

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

