#include "visualization/layout.hpp"
#include <algorithm>
#include <cmath>
#include <unordered_map>
#define _USE_MATH_DEFINES
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace graph {

void Layout::applyLayout(Graph& g, LayoutType type, double width, double height) {
    switch (type) {
        case LayoutType::Circular:
            circular(g, width, height);
            break;
        case LayoutType::ForceDirected:
            forceDirected(g, width, height);
            break;
        case LayoutType::Random:
            random(g, width, height);
            break;
    }
}

void Layout::circular(Graph& g, double width, double height) {
    auto vertices = g.getVertices();
    int n = static_cast<int>(vertices.size());
    if (n == 0) return;
    
    double centerX = width / 2.0;
    double centerY = height / 2.0;
    double radius = std::min(width, height) * 0.4;
    
    double angleStep = 2.0 * M_PI / n;
    
    for (int i = 0; i < n; ++i) {
        double angle = i * angleStep;
        double x = centerX + radius * std::cos(angle);
        double y = centerY + radius * std::sin(angle);
        g.setVertexPosition(vertices[i], x, y);
    }
}

void Layout::random(Graph& g, double width, double height) {
    std::uniform_real_distribution<double> xDist(50.0, width - 50.0);
    std::uniform_real_distribution<double> yDist(50.0, height - 50.0);
    
    auto vertices = g.getVertices();
    for (int v : vertices) {
        g.setVertexPosition(v, xDist(gen_), yDist(gen_));
    }
}

void Layout::forceDirected(Graph& g, double width, double height, int iterations) {
    auto vertices = g.getVertices();
    int n = static_cast<int>(vertices.size());
    if (n == 0) return;
    
    // Инициализация случайными позициями
    random(g, width, height);
    
    // Убедиться, что координаты установлены
    for (int v : vertices) {
        auto* vertex = g.getVertex(v);
        if (vertex && (vertex->x == 0.0 && vertex->y == 0.0)) {
            // Если координаты не установлены, установить случайные
            std::uniform_real_distribution<double> xDist(50.0, width - 50.0);
            std::uniform_real_distribution<double> yDist(50.0, height - 50.0);
            g.setVertexPosition(v, xDist(gen_), yDist(gen_));
        }
    }
    
    // Параметры алгоритма
    double k = std::sqrt((width * height) / n);  // Идеальное расстояние
    double temperature = std::min(width, height) / 10.0;
    
    for (int iter = 0; iter < iterations; ++iter) {
        std::unordered_map<int, std::pair<double, double>> forces;
        
        // Инициализировать силы
        for (int v : vertices) {
            forces[v] = {0.0, 0.0};
        }
        
        // Вычислить силы отталкивания между всеми парами вершин
        for (size_t i = 0; i < vertices.size(); ++i) {
            int v1 = vertices[i];
            auto* vertex1 = g.getVertex(v1);
            if (!vertex1) continue;
            
            for (size_t j = i + 1; j < vertices.size(); ++j) {
                int v2 = vertices[j];
                auto* vertex2 = g.getVertex(v2);
                if (!vertex2) continue;
                
                double dx = vertex2->x - vertex1->x;
                double dy = vertex2->y - vertex1->y;
                double dist = distance(vertex1->x, vertex1->y, vertex2->x, vertex2->y);
                
                if (dist < 0.01) dist = 0.01;  // Избежать деления на ноль
                
                // Сила отталкивания
                double repulsion = k * k / dist;
                double fx = (dx / dist) * repulsion;
                double fy = (dy / dist) * repulsion;
                
                forces[v1].first -= fx;
                forces[v1].second -= fy;
                forces[v2].first += fx;
                forces[v2].second += fy;
            }
        }
        
        // Вычислить силы притяжения для рёбер
        auto edges = g.getEdges();
        for (const auto& edge : edges) {
            auto* v1 = g.getVertex(edge.from);
            auto* v2 = g.getVertex(edge.to);
            if (!v1 || !v2) continue;
            
            double dx = v2->x - v1->x;
            double dy = v2->y - v1->y;
            double dist = distance(v1->x, v1->y, v2->x, v2->y);
            
            if (dist < 0.01) dist = 0.01;
            
            // Сила притяжения
            double attraction = dist * dist / k;
            double fx = (dx / dist) * attraction;
            double fy = (dy / dist) * attraction;
            
            forces[edge.from].first += fx;
            forces[edge.from].second += fy;
            forces[edge.to].first -= fx;
            forces[edge.to].second -= fy;
        }
        
        // Применить силы с ограничением температуры
        for (int v : vertices) {
            double fx = forces[v].first;
            double fy = forces[v].second;
            double forceMag = std::sqrt(fx * fx + fy * fy);
            
            if (forceMag > temperature) {
                fx = (fx / forceMag) * temperature;
                fy = (fy / forceMag) * temperature;
            }
            
            auto* vertex = g.getVertex(v);
            if (vertex) {
                double newX = vertex->x + fx;
                double newY = vertex->y + fy;
                
                // Ограничить границами
                newX = std::max(50.0, std::min(width - 50.0, newX));
                newY = std::max(50.0, std::min(height - 50.0, newY));
                
                g.setVertexPosition(v, newX, newY);
            }
        }
        
        // Охлаждение
        temperature *= 0.95;
    }
}

void Layout::updateForceDirected(Graph& g, double width, double height, int iterations) {
    forceDirected(g, width, height, iterations);
}

} // namespace graph

