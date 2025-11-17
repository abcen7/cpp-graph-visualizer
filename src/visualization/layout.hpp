#pragma once

#include "core/graph.hpp"
#include <vector>
#include <random>
#include <cmath>

namespace graph {

enum class LayoutType {
    Circular,
    ForceDirected,
    Random
};

class Layout {
public:
    Layout() : gen_(std::random_device{}()) {}
    
    // Применить макет к графу
    void applyLayout(Graph& g, LayoutType type, double width = 800.0, double height = 600.0);
    
    // Force directed layout Fruchterman Reingold
    void forceDirected(Graph& g, double width, double height, int iterations = 100);
    
    // Circular layout
    void circular(Graph& g, double width, double height);
    
    // Random layout
    void random(Graph& g, double width, double height);
    
    // Обновление force directed для анимации
    void updateForceDirected(Graph& g, double width, double height, int iterations = 1);
    
private:
    std::mt19937 gen_;
    
    double distance(double x1, double y1, double x2, double y2) const {
        return std::sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
    }
};

} // namespace graph

