#pragma once

#include "core/graph.hpp"
#include "core/algorithms.hpp"
#include <SFML/Graphics.hpp>
#include <memory>
#include <string>

namespace graph {

class GraphRenderer {
public:
    GraphRenderer(sf::RenderWindow& window);
    
    // Отрисовка графа
    void render(const Graph& g, const AlgorithmState& state, sf::RenderTarget& target);
    
    // Обработка событий мыши для масштабирования и панорамирования
    void handleMouseWheel(float delta);
    void handleMouseDrag(sf::Vector2f delta);
    void resetView();
    
    // Получить выбранную вершину по координатам
    int getVertexAt(sf::Vector2f position, const Graph& g) const;
    
    // Настройки отрисовки
    void setVertexRadius(float radius) { vertexRadius_ = radius; }
    void setEdgeWidth(float width) { edgeWidth_ = width; }
    void setAnimationSpeed(float speed) { animationSpeed_ = speed; }
    
private:
    sf::RenderWindow& window_;
    sf::View view_;
    
    float vertexRadius_ = 15.0f;
    float edgeWidth_ = 2.0f;
    float animationSpeed_ = 1.0f;
    float zoom_ = 1.0f;
    sf::Vector2f panOffset_{0.0f, 0.0f};
    
    // Цвета
    sf::Color vertexColor_ = sf::Color::White;
    sf::Color edgeColor_ = sf::Color(100, 100, 100);
    sf::Color visitedColor_ = sf::Color::Green;
    sf::Color currentColor_ = sf::Color::Red;
    sf::Color pathColor_ = sf::Color::Blue;
    
    void drawEdge(const Graph& g, int from, int to, const sf::Color& color, sf::RenderTarget& target);
    void drawVertex(const Graph& g, int id, const sf::Color& color, sf::RenderTarget& target);
    void drawLabel(const Graph& g, int id, sf::RenderTarget& target);
};

} // namespace graph

