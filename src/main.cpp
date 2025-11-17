#include <SFML/Graphics.hpp>
#include <iostream>
#include <memory>
#include <thread>
#include <future>
#include <string>
#include <variant>
#include <type_traits>
#include <optional>
#include <SFML/Window/Event.hpp>

#include "core/graph.hpp"
#include "core/algorithms.hpp"
#include "core/parallel.hpp"
#include "io/loader.hpp"
#include "visualization/layout.hpp"
#include "visualization/renderer.hpp"

using namespace graph;

enum class AlgorithmType {
    None,
    BFS,
    DFS,
    Dijkstra,
    ParallelBFS
};

class GraphVisualizerApp {
public:
    GraphVisualizerApp(const std::string& graphFile = "") 
        : algorithmType_(AlgorithmType::None),
          selectedStartVertex_(-1),
          selectedEndVertex_(-1),
          animationSpeed_(1.0f) {
        
        std::cout << "[DEBUG] Начало конструктора GraphVisualizerApp" << std::endl;
        std::cout.flush();
        
        // Настройки контекста OpenGL для SFML 3
        sf::ContextSettings settings;
        settings.antiAliasingLevel = 8;  // SFML 3: camelCase
        settings.depthBits = 24;
        settings.stencilBits = 8;
        
        // Создать окно с настройками контекста (SFML 3)
        // В SFML 3 RenderWindow создается через create с правильной сигнатурой
        // Сигнатура: create(VideoMode, String, State, ContextSettings)
        window_.create(
            sf::VideoMode({1200u, 800u}),
            "Graph Visualizer",
            sf::State::Windowed,
            settings
        );
        
        // Проверить, что окно создано правильно
        if (!window_.isOpen()) {
            std::cerr << "Ошибка: окно не создано!" << std::endl;
            std::cerr.flush();
            return;
        }
        
        std::cout << "[DEBUG] Окно создано успешно" << std::endl;
        std::cout.flush();
        
        // Установить размер view сразу после создания окна
        sf::View initialView(sf::FloatRect({0, 0}, {1200.0f, 800.0f}));
        window_.setView(initialView);
        
        std::cout << "[DEBUG] View установлен" << std::endl;
        std::cout.flush();
        
        // Инициализировать renderer после создания окна
        renderer_ = std::make_unique<GraphRenderer>(window_);
        
        std::cout << "[DEBUG] Renderer создан" << std::endl;
        std::cout.flush();
        
        // Установить непрозрачность окна (важно для macOS)
        (void)window_.setActive(true);
        
        // Убедиться, что окно видимо и активно
        window_.requestFocus();
        
        std::cout << "Окно создано. Размер: " << window_.getSize().x << "x" << window_.getSize().y << std::endl;
        std::cout.flush();
        
        // Загрузить граф из файла, если указан
        if (!graphFile.empty()) {
            std::cout << "[DEBUG] Загрузка графа из файла: " << graphFile << std::endl;
            std::cout.flush();
            loadGraphFromFile(graphFile);
        }
        
        // Создать тестовый граф, если не загружен из файла
        if (!graph_) {
            std::cout << "[DEBUG] Создание тестового графа" << std::endl;
            std::cout.flush();
            try {
                createTestGraph();
            } catch (const std::exception& e) {
                std::cerr << "[ERROR] Исключение при создании тестового графа: " << e.what() << std::endl;
                std::cerr.flush();
            }
        }
        
        std::cout << "[DEBUG] После createTestGraph(), graph_ = " << (graph_ ? "существует" : "null") << std::endl;
        std::cout.flush();
        
        std::cout << "[DEBUG] Применение layout" << std::endl;
        std::cout.flush();
        try {
            applyLayout(LayoutType::ForceDirected);
            std::cout << "[DEBUG] Layout применён" << std::endl;
            std::cout.flush();
        } catch (const std::exception& e) {
            std::cerr << "[ERROR] Исключение при применении layout: " << e.what() << std::endl;
            std::cerr.flush();
        }
        
        // Отладочный вывод
        std::cout << "[DEBUG] graph_ " << (graph_ ? "существует" : "null") << std::endl;
        std::cout.flush();
        if (graph_) {
            std::cout << "[DEBUG] Вершин: " << graph_->getVertexCount() 
                     << ", Рёбер: " << graph_->getEdgeCount() << std::endl;
            auto vertices = graph_->getVertices();
            if (!vertices.empty()) {
                std::cout << "[DEBUG] ID вершин: ";
                for (int id : vertices) {
                    std::cout << id << " ";
                }
                std::cout << std::endl;
                auto* v = graph_->getVertex(vertices[0]);
                if (v) {
                    std::cout << "[DEBUG] Координаты первой вершины (" << vertices[0] << "): (" 
                             << v->x << ", " << v->y << ")" << std::endl;
                }
            } else {
                std::cout << "[DEBUG] ВНИМАНИЕ: граф существует, но вершин нет!" << std::endl;
            }
        } else {
            std::cout << "[DEBUG] ВНИМАНИЕ: graph_ == nullptr!" << std::endl;
        }
        
        // Принудительная отрисовка при инициализации для macOS
        std::cout << "[DEBUG] Вызов render()" << std::endl;
        std::cout.flush();
        render();
        
        std::cout << "[DEBUG] Конструктор завершён" << std::endl;
        std::cout.flush();
    }
    
    void run() {
        std::cout << "[DEBUG] Запуск главного цикла run()" << std::endl;
        std::cout.flush();
        while (window_.isOpen()) {
            handleEvents();
            update();
            render();
        }
        std::cout << "[DEBUG] Главный цикл завершён" << std::endl;
        std::cout.flush();
    }
    
private:
    sf::RenderWindow window_;
    std::unique_ptr<Graph> graph_;
    std::unique_ptr<GraphRenderer> renderer_;
    Layout layout_;
    AlgorithmState algorithmState_;
    
    AlgorithmType algorithmType_;
    int selectedStartVertex_;
    int selectedEndVertex_;
    float animationSpeed_;
    LayoutType currentLayout_ = LayoutType::ForceDirected;
    
    std::future<std::vector<int>> algorithmFuture_;
    bool isAlgorithmRunning_ = false;
    bool isDragging_ = false;
    sf::Vector2i lastMousePos_;
    
    void handleEvents() {
        // SFML 3: события обрабатываются через pollEvent с variant
        while (const std::optional<sf::Event> event = window_.pollEvent()) {
            // SFML 3: используем методы is<>() и get<>() для обработки событий
            if (event->is<sf::Event::Closed>()) {
                window_.close();
            }
            else if (const auto* mouseWheel = event->getIf<sf::Event::MouseWheelScrolled>()) {
                if (renderer_) renderer_->handleMouseWheel(mouseWheel->delta);
            }
            else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                handleKeyPress(keyPressed->code);
            }
            else if (const auto* mouseButton = event->getIf<sf::Event::MouseButtonPressed>()) {
                handleMouseClick(*mouseButton);
                if (mouseButton->button == sf::Mouse::Button::Right) {
                    isDragging_ = true;
                    lastMousePos_ = mouseButton->position;
                }
            }
            else if (const auto* mouseReleased = event->getIf<sf::Event::MouseButtonReleased>()) {
                if (mouseReleased->button == sf::Mouse::Button::Right) {
                    isDragging_ = false;
                }
            }
            else if (const auto* mouseMoved = event->getIf<sf::Event::MouseMoved>()) {
                if (isDragging_) {
                    sf::Vector2f delta(
                        static_cast<float>(mouseMoved->position.x - lastMousePos_.x),
                        static_cast<float>(mouseMoved->position.y - lastMousePos_.y)
                    );
                    if (renderer_) renderer_->handleMouseDrag(delta);
                    lastMousePos_ = mouseMoved->position;
                }
            }
        }
    }
    
    void handleKeyPress(sf::Keyboard::Key key) {
        switch (key) {
            case sf::Keyboard::Key::L:
                loadGraph();
                break;
            case sf::Keyboard::Key::S:
                saveGraph();
                break;
            case sf::Keyboard::Key::B:
                startAlgorithm(AlgorithmType::BFS);
                break;
            case sf::Keyboard::Key::D:
                startAlgorithm(AlgorithmType::DFS);
                break;
            case sf::Keyboard::Key::I:  // Dijkstra
                startAlgorithm(AlgorithmType::Dijkstra);
                break;
            case sf::Keyboard::Key::P:
                startAlgorithm(AlgorithmType::ParallelBFS);
                break;
            case sf::Keyboard::Key::Space:
                pauseResumeAlgorithm();
                break;
            case sf::Keyboard::Key::R:
                resetAlgorithm();
                break;
            case sf::Keyboard::Key::C:
                applyLayout(LayoutType::Circular);
                break;
            case sf::Keyboard::Key::F:
                applyLayout(LayoutType::ForceDirected);
                break;
            case sf::Keyboard::Key::N:  // Random
                applyLayout(LayoutType::Random);
                break;
            case sf::Keyboard::Key::Escape:
                window_.close();
                break;
            default:
                break;
        }
    }
    
    void handleMouseClick(const sf::Event::MouseButtonPressed& mouseEvent) {
        if (mouseEvent.button == sf::Mouse::Button::Left) {
            // Преобразовать координаты экрана в координаты мира (SFML 3)
            sf::Vector2f mousePos = window_.mapPixelToCoords(mouseEvent.position);
            int vertexId = renderer_ ? renderer_->getVertexAt(mousePos, *graph_) : -1;
            
            if (vertexId != -1) {
                if (selectedStartVertex_ == -1) {
                    selectedStartVertex_ = vertexId;
                    std::cout << "Выбрана начальная вершина: " << vertexId << std::endl;
                } else if (selectedEndVertex_ == -1 && algorithmType_ == AlgorithmType::Dijkstra) {
                    selectedEndVertex_ = vertexId;
                    std::cout << "Выбрана конечная вершина: " << vertexId << std::endl;
                    startAlgorithm(AlgorithmType::Dijkstra);
                } else {
                    selectedStartVertex_ = vertexId;
                    selectedEndVertex_ = -1;
                    std::cout << "Выбрана новая начальная вершина: " << vertexId << std::endl;
                }
            }
        }
    }
    
    void startAlgorithm(AlgorithmType type) {
        if (isAlgorithmRunning_) {
            std::cout << "Алгоритм уже выполняется" << std::endl;
            return;
        }
        
        if (selectedStartVertex_ == -1) {
            auto vertices = graph_->getVertices();
            if (vertices.empty()) {
                std::cout << "Граф пуст" << std::endl;
                return;
            }
            selectedStartVertex_ = vertices[0];
        }
        
        algorithmType_ = type;
        isAlgorithmRunning_ = true;
        
        if (type == AlgorithmType::Dijkstra) {
            if (selectedEndVertex_ == -1) {
                std::cout << "Для Dijkstra нужна конечная вершина. Кликните на вершину." << std::endl;
                isAlgorithmRunning_ = false;
                return;
            }
            algorithmFuture_ = std::async(std::launch::async, 
                [this]() {
                    return Algorithms::Dijkstra(*graph_, selectedStartVertex_, 
                                               selectedEndVertex_, algorithmState_);
                });
        } else if (type == AlgorithmType::BFS) {
            algorithmFuture_ = std::async(std::launch::async,
                [this]() {
                    return Algorithms::BFS(*graph_, selectedStartVertex_, algorithmState_);
                });
        } else if (type == AlgorithmType::DFS) {
            algorithmFuture_ = std::async(std::launch::async,
                [this]() {
                    return Algorithms::DFS(*graph_, selectedStartVertex_, algorithmState_);
                });
        } else if (type == AlgorithmType::ParallelBFS) {
            algorithmFuture_ = std::async(std::launch::async,
                [this]() {
                    return ParallelAlgorithms::parallelBFS(*graph_, selectedStartVertex_, 
                                                          algorithmState_, 4);
                });
        }
    }
    
    void pauseResumeAlgorithm() {
        if (isAlgorithmRunning_) {
            algorithmState_.isPaused = !algorithmState_.isPaused;
            std::cout << (algorithmState_.isPaused ? "Пауза" : "Продолжение") << std::endl;
        }
    }
    
    void resetAlgorithm() {
        algorithmState_.reset();
        isAlgorithmRunning_ = false;
        selectedStartVertex_ = -1;
        selectedEndVertex_ = -1;
        std::cout << "Алгоритм сброшен" << std::endl;
    }
    
    void update() {
        // Проверить завершение алгоритма
        if (isAlgorithmRunning_ && algorithmFuture_.valid()) {
            auto status = algorithmFuture_.wait_for(std::chrono::milliseconds(0));
            if (status == std::future_status::ready) {
                auto result = algorithmFuture_.get();
                isAlgorithmRunning_ = false;
                std::cout << "Алгоритм завершен. Обработано вершин: " << result.size() << std::endl;
            }
        }
        
        // Force directed layout обновляется только при изменении графа или при явном запросе
        // Убрано постоянное обновление для стабильности визуализации
        // Если нужна анимация, можно добавить флаг и обновлять с ограничением частоты
    }
    
    void render() {
        // Убедиться, что окно активно
        (void)window_.setActive(true);
        
        // Очистить окно непрозрачным цветом (важно для macOS)
        window_.clear(sf::Color(60, 60, 60, 255));
        
        // Установить view для окна (если не установлен)
        auto currentView = window_.getView();
        if (currentView.getSize().x == 0 || currentView.getSize().y == 0) {
            sf::View defaultView(sf::FloatRect({0, 0}, {1200.0f, 800.0f}));
            window_.setView(defaultView);
        }
        
        // Всегда отрисовать непрозрачный фоновый прямоугольник
        sf::RectangleShape background(sf::Vector2f(1200.0f, 800.0f));
        background.setPosition({0.0f, 0.0f});
        background.setFillColor(sf::Color(80, 80, 80, 255));
        window_.draw(background);
        
        if (graph_) {
            // Проверить, что граф не пустой
            if (graph_->getVertexCount() > 0 && renderer_) {
                renderer_->render(*graph_, algorithmState_, window_);
            } else {
                // Отладочная отрисовка для пустого графа
                sf::CircleShape testCircle(50.0f);
                testCircle.setPosition({100.0f, 100.0f});
                testCircle.setFillColor(sf::Color::Red);
                window_.draw(testCircle);
            }
        } else {
            // Отладочная отрисовка для неинициализированного графа
            sf::CircleShape testCircle(50.0f);
            testCircle.setPosition({100.0f, 100.0f});
            testCircle.setFillColor(sf::Color::Blue);
            window_.draw(testCircle);
        }
        
        // Отрисовать информацию
        renderInfo();
        
        // Отобразить содержимое окна
        window_.display();
    }
    
    void renderInfo() {
        // Простой текстовый вывод в консоль
        // Для полноценного UI можно использовать ImGui
    }
    
    void applyLayout(LayoutType type) {
        if (!graph_) return;
        currentLayout_ = type;
        layout_.applyLayout(*graph_, type, 1200.0, 800.0);
    }
    
    void loadGraph() {
        // Для интерактивной загрузки используйте примеры из папки examples
        // или укажите файл при запуске программы
        std::cout << "Используйте аргумент командной строки для загрузки графа" << std::endl;
        std::cout << "Пример: ./GraphVisualizer examples/test_graph.csv" << std::endl;
    }
    
    void loadGraphFromFile(const std::string& filename) {
        std::unique_ptr<Graph> newGraph;
        if (filename.find(".csv") != std::string::npos) {
            newGraph = GraphLoader::loadFromCSV(filename, false);
        } else if (filename.find(".json") != std::string::npos) {
            newGraph = GraphLoader::loadFromJSON(filename, false);
        } else {
            std::cout << "Неподдерживаемый формат файла: " << filename << std::endl;
            return;
        }
        
        if (newGraph) {
            graph_ = std::move(newGraph);
            applyLayout(currentLayout_);
            std::cout << "Граф загружен из " << filename << std::endl;
            std::cout << "Вершин: " << graph_->getVertexCount() 
                     << ", Рёбер: " << graph_->getEdgeCount() << std::endl;
        } else {
            std::cout << "Не удалось загрузить граф из " << filename << std::endl;
        }
    }
    
    void saveGraph() {
        if (!graph_) {
            std::cout << "Нет графа для сохранения" << std::endl;
            return;
        }
        
        // Сохранить в текущую директорию с именем по умолчанию
        std::string filename = "saved_graph.json";
        bool success = GraphLoader::saveToJSON(*graph_, filename);
        
        if (success) {
            std::cout << "Граф сохранен в " << filename << std::endl;
        } else {
            std::cout << "Ошибка при сохранении" << std::endl;
        }
    }
    
    void createTestGraph() {
        std::cout << "[DEBUG] createTestGraph() начат" << std::endl;
        std::cout.flush();
        
        try {
            graph_ = std::make_unique<Graph>(false);
            std::cout << "[DEBUG] Graph создан" << std::endl;
            std::cout.flush();
            
            // Создать тестовый граф
            std::cout << "[DEBUG] Добавление рёбер..." << std::endl;
            std::cout.flush();
            
            graph_->addEdge(1, 2, 1.0);
            std::cout << "[DEBUG] Ребро 1->2 добавлено" << std::endl;
            std::cout.flush();
            
            graph_->addEdge(2, 3, 2.0);
            graph_->addEdge(3, 4, 1.5);
            graph_->addEdge(4, 5, 1.0);
            graph_->addEdge(5, 1, 2.5);
            graph_->addEdge(2, 5, 1.0);
            graph_->addEdge(3, 6, 1.0);
            graph_->addEdge(6, 7, 1.0);
            graph_->addEdge(7, 8, 1.0);
            graph_->addEdge(8, 6, 1.0);
            
            std::cout << "[DEBUG] Все рёбра добавлены. Вершин: " << graph_->getVertexCount() 
                     << ", Рёбер: " << graph_->getEdgeCount() << std::endl;
            std::cout.flush();
        } catch (const std::exception& e) {
            std::cerr << "[ERROR] Исключение в createTestGraph(): " << e.what() << std::endl;
            std::cerr.flush();
        } catch (...) {
            std::cerr << "[ERROR] Неизвестное исключение в createTestGraph()" << std::endl;
            std::cerr.flush();
        }
        
        std::cout << "[DEBUG] createTestGraph() завершён" << std::endl;
        std::cout.flush();
    }
};

int main(int argc, char* argv[]) {
    std::string graphFile = "";
    if (argc > 1) {
        graphFile = argv[1];
    }
    
    GraphVisualizerApp app(graphFile);
    app.run();
    return 0;
}

