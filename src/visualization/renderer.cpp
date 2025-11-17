#include "visualization/renderer.hpp"
#include <cmath>
#include <algorithm>
#include <iostream>

namespace graph {

GraphRenderer::GraphRenderer(sf::RenderWindow& window) 
    : window_(window) {
    view_ = sf::View(sf::FloatRect({0, 0}, {static_cast<float>(window_.getSize().x), static_cast<float>(window_.getSize().y)}));
}

void GraphRenderer::render(const Graph& g, const AlgorithmState& state, sf::RenderTarget& target) {
    // Отладочный вывод
    static int renderCallCount = 0;
    renderCallCount++;
    if (renderCallCount <= 3) {
        std::cout << "[Renderer] render() вызван #" << renderCallCount 
                  << ", вершин в графе: " << g.getVertexCount() << std::endl;
    }
    
    // Убедиться, что view правильно настроен
    if (view_.getSize().x == 0 || view_.getSize().y == 0) {
        view_ = sf::View(sf::FloatRect({0, 0}, {static_cast<float>(window_.getSize().x), static_cast<float>(window_.getSize().y)}));
    }
    
    // Сохранить текущий view и установить наш
    sf::View originalView = target.getView();
    target.setView(view_);
    
    // Отрисовать рёбра
    auto edges = g.getEdges();
    for (const auto& edge : edges) {
        bool isInPath = std::find(state.path.begin(), state.path.end(), edge.from) != state.path.end() &&
                       std::find(state.path.begin(), state.path.end(), edge.to) != state.path.end();
        
        // Проверить, является ли ребро частью пути
        bool isPathEdge = false;
        if (isInPath) {
            auto fromIt = std::find(state.path.begin(), state.path.end(), edge.from);
            auto toIt = std::find(state.path.begin(), state.path.end(), edge.to);
            if (fromIt != state.path.end() && toIt != state.path.end()) {
                int fromIdx = std::distance(state.path.begin(), fromIt);
                int toIdx = std::distance(state.path.begin(), toIt);
                isPathEdge = std::abs(fromIdx - toIdx) == 1;
            }
        }
        
        sf::Color edgeColor = isPathEdge ? pathColor_ : edgeColor_;
        drawEdge(g, edge.from, edge.to, edgeColor, target);
    }
    
    // Отрисовать вершины
    auto vertices = g.getVertices();
    for (int id : vertices) {
        bool isVisited = std::find(state.visited.begin(), state.visited.end(), id) != state.visited.end();
        bool isCurrent = (state.currentVertex == id);
        bool isInPath = std::find(state.path.begin(), state.path.end(), id) != state.path.end();
        
        sf::Color color = vertexColor_;
        if (isCurrent) {
            color = currentColor_;
        } else if (isInPath) {
            color = pathColor_;
        } else if (isVisited) {
            color = visitedColor_;
        }
        
        drawVertex(g, id, color, target);
        drawLabel(g, id, target);
    }
    
    // Восстановить оригинальный view
    target.setView(originalView);
}

void GraphRenderer::drawEdge(const Graph& g, int from, int to, const sf::Color& color, sf::RenderTarget& target) {
    const Vertex* v1 = g.getVertex(from);
    const Vertex* v2 = g.getVertex(to);
    if (!v1 || !v2) return;
    
    sf::VertexArray lineArray(sf::PrimitiveType::Lines, 2);
    lineArray[0].position = sf::Vector2f(static_cast<float>(v1->x), static_cast<float>(v1->y));
    lineArray[0].color = color;
    lineArray[1].position = sf::Vector2f(static_cast<float>(v2->x), static_cast<float>(v2->y));
    lineArray[1].color = color;
    
    target.draw(lineArray);
}

void GraphRenderer::drawVertex(const Graph& g, int id, const sf::Color& color, sf::RenderTarget& target) {
    const Vertex* v = g.getVertex(id);
    if (!v) {
        std::cerr << "[Renderer] Вершина " << id << " не найдена!" << std::endl;
        return;
    }
    
    // Отладочный вывод для первых нескольких вершин
    static int debugCount = 0;
    if (debugCount < 3) {
        std::cout << "[Renderer] Отрисовка вершины " << id 
                  << " в позиции (" << v->x << ", " << v->y << ")" << std::endl;
        debugCount++;
    }
    
    sf::CircleShape circle(vertexRadius_);
    circle.setPosition({static_cast<float>(v->x) - vertexRadius_, 
                       static_cast<float>(v->y) - vertexRadius_});
    circle.setFillColor(color);
    circle.setOutlineColor(sf::Color::Black);
    circle.setOutlineThickness(2.0f);
    
    target.draw(circle);
}

void GraphRenderer::drawLabel(const Graph& g, int id, sf::RenderTarget& target) {
    const Vertex* v = g.getVertex(id);
    if (!v) return;
    
    (void)target;  // Пока не используется
}

void GraphRenderer::handleMouseWheel(float delta) {
    float zoomFactor = 1.0f + delta * 0.1f;
    view_.zoom(1.0f / zoomFactor);
    zoom_ *= zoomFactor;
}

void GraphRenderer::handleMouseDrag(sf::Vector2f delta) {
    view_.move(-delta);
    panOffset_ += delta;
}

void GraphRenderer::resetView() {
    view_ = sf::View(sf::FloatRect({0, 0}, {static_cast<float>(window_.getSize().x), static_cast<float>(window_.getSize().y)}));
    zoom_ = 1.0f;
    panOffset_ = sf::Vector2f(0.0f, 0.0f);
}

int GraphRenderer::getVertexAt(sf::Vector2f position, const Graph& g) const {
    auto vertices = g.getVertices();
    for (int id : vertices) {
        const Vertex* v = g.getVertex(id);
        if (!v) continue;
        
        float dx = position.x - static_cast<float>(v->x);
        float dy = position.y - static_cast<float>(v->y);
        float dist = std::sqrt(dx * dx + dy * dy);
        
        if (dist <= vertexRadius_) {
            return id;
        }
    }
    return -1;
}

} // namespace graph

