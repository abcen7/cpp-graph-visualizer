# Graph Visualizer

Многопоточное приложение для обработки и визуализации графов на C++.

## Требования

- C++17 или выше
- CMake 3.15+
- SFML 2.5+

## Сборка

### Быстрая сборка (релизная версия)

**macOS/Linux:**
```bash
./build-release.sh
```

**Windows:**
```cmd
build-release-windows.bat
```

### Ручная сборка

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

Подробные инструкции см. в [BUILD.md](BUILD.md) и [BUILD-WINDOWS.md](BUILD-WINDOWS.md)

## Использование

### Запуск приложения

```bash
# Запуск с тестовым графом
./GraphVisualizer

# Запуск с загрузкой графа из файла
./GraphVisualizer examples/test_graph.csv
./GraphVisualizer examples/test_graph.json
```

### Управление

**Клавиатура:**
- `L` - Загрузить граф из файла (CSV или JSON)
- `S` - Сохранить граф в файл
- `B` - Запустить BFS (обход в ширину)
- `D` - Запустить DFS (обход в глубину)
- `I` - Запустить алгоритм Dijkstra (поиск кратчайшего пути)
- `P` - Запустить параллельный BFS
- `Space` - Пауза/продолжение алгоритма
- `R` - Сброс алгоритма
- `C` - Применить круговой макет
- `F` - Применить force-directed макет
- `N` - Применить случайный макет
- `Esc` - Выход

**Мышь:**
- Колесо мыши - Масштабирование
- Левый клик на вершине - Выбрать начальную вершину
- Для Dijkstra: кликните на начальную вершину, затем на конечную
- Правая кнопка мыши + перетаскивание - Панорамирование графа

### Формат файлов

**CSV формат:**
```
from,to,weight
1,2,5
2,3,7
3,4,1
```

**JSON формат:**
```json
{
  "directed": false,
  "edges": [
    { "from": 1, "to": 2, "weight": 5 },
    { "from": 2, "to": 3, "weight": 7 },
    { "from": 3, "to": 4, "weight": 1 }
  ]
}
```

## Архитектура

```
src/
├── core/
│   ├── graph.hpp/cpp          # Структура данных графа
│   ├── algorithms.hpp/cpp      # BFS, DFS, Dijkstra
│   └── parallel.hpp/cpp        # Многопоточная обработка
├── io/
│   └── loader.hpp/cpp          # Загрузка/сохранение графа
├── visualization/
│   ├── layout.hpp/cpp          # Алгоритмы позиционирования
│   └── renderer.hpp/cpp         # Отрисовка с SFML
└── main.cpp                     # Точка входа
```

## Особенности

- Потокобезопасная структура данных графа
- Параллельная обработка алгоритмов
- Интерактивная визуализация с подсветкой активных вершин
- Поддержка различных макетов графа
- Загрузка и сохранение графов в CSV и JSON форматах

## Инструкции по сборке проекта

### Требования

- C++20 компилятор (GCC, Clang, или MSVC)
- CMake 3.15+
- SFML 3.0+ (установлен и доступен для CMake)

1. Установите зависимости:
   ```bash
   brew install sfml cmake
   ```

2. Соберите проект:
   ```bash
   mkdir build
   cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release
   cmake --build . --config Release
   ```

3. Запустите:
   ```bash
   ./GraphVisualizer
   ```