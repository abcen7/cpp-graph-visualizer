#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <mutex>
#include <atomic>

namespace graph {

struct Vertex {
    int id;
    double x, y;  // координаты для визуализации
    std::string label;
    
    Vertex(int id = 0, double x = 0.0, double y = 0.0, const std::string& label = "")
        : id(id), x(x), y(y), label(label) {}
};

struct Edge {
    int from;
    int to;
    double weight;
    bool directed;
    
    Edge(int from, int to, double weight = 1.0, bool directed = false)
        : from(from), to(to), weight(weight), directed(directed) {}
    
    bool operator==(const Edge& other) const {
        return from == other.from && to == other.to;
    }
};

class Graph {
public:
    Graph(bool directed = false);
    
    // Добавление вершин и рёбер
    void addVertex(int id, const std::string& label = "");
    void addEdge(int from, int to, double weight = 1.0);
    void removeVertex(int id);
    void removeEdge(int from, int to);
    
    // Получение информации
    bool hasVertex(int id) const;
    bool hasEdge(int from, int to) const;
    std::vector<int> getNeighbors(int id) const;
    std::vector<Edge> getEdges() const;
    std::vector<int> getVertices() const;
    int getVertexCount() const;
    int getEdgeCount() const;
    bool isDirected() const;
    
    // Получение вершин и рёбер
    Vertex* getVertex(int id);
    const Vertex* getVertex(int id) const;
    Edge getEdge(int from, int to) const;
    
    // Установка координат
    void setVertexPosition(int id, double x, double y);
    
    // Вычисление характеристик
    int getDegree(int id) const;
    double getDensity() const;
    std::vector<std::vector<int>> getConnectedComponents() const;
    
    // Потокобезопасный доступ
    std::mutex& getMutex() const { return mutex_; }
    
private:
    bool directed_;
    std::unordered_map<int, std::unique_ptr<Vertex>> vertices_;
    std::unordered_map<int, std::vector<Edge>> adjacency_list_;
    mutable std::mutex mutex_;
    
    void addEdgeInternal(int from, int to, double weight);
    void addVertexInternal(int id, const std::string& label = "");  // Без блокировки
};

} // namespace graph

