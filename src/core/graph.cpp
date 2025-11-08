#include "core/graph.hpp"
#include <algorithm>
#include <cmath>
#include <unordered_set>
#include <functional>

namespace graph {

Graph::Graph(bool directed) : directed_(directed) {}

void Graph::addVertex(int id, const std::string& label) {
    std::lock_guard<std::mutex> lock(mutex_);
    addVertexInternal(id, label);
}

void Graph::addVertexInternal(int id, const std::string& label) {
    if (vertices_.find(id) == vertices_.end()) {
        vertices_[id] = std::make_unique<Vertex>(id, 0.0, 0.0, label);
        adjacency_list_[id] = std::vector<Edge>();
    }
}

void Graph::addEdge(int from, int to, double weight) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (vertices_.find(from) == vertices_.end()) {
        addVertexInternal(from);
    }
    if (vertices_.find(to) == vertices_.end()) {
        addVertexInternal(to);
    }
    addEdgeInternal(from, to, weight);
    if (!directed_) {
        addEdgeInternal(to, from, weight);
    }
}

void Graph::addEdgeInternal(int from, int to, double weight) {
    Edge edge(from, to, weight, directed_);
    auto& edges = adjacency_list_[from];
    auto it = std::find_if(edges.begin(), edges.end(),
        [&](const Edge& e) { return e.to == to; });
    if (it == edges.end()) {
        edges.push_back(edge);
    } else {
        it->weight = weight;
    }
}

void Graph::removeVertex(int id) {
    std::lock_guard<std::mutex> lock(mutex_);
    vertices_.erase(id);
    adjacency_list_.erase(id);
    for (auto& [vid, edges] : adjacency_list_) {
        edges.erase(
            std::remove_if(edges.begin(), edges.end(),
                [id](const Edge& e) { return e.to == id; }),
            edges.end());
    }
}

void Graph::removeEdge(int from, int to) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (adjacency_list_.find(from) != adjacency_list_.end()) {
        auto& edges = adjacency_list_[from];
        edges.erase(
            std::remove_if(edges.begin(), edges.end(),
                [to](const Edge& e) { return e.to == to; }),
            edges.end());
    }
    if (!directed_ && adjacency_list_.find(to) != adjacency_list_.end()) {
        auto& edges = adjacency_list_[to];
        edges.erase(
            std::remove_if(edges.begin(), edges.end(),
                [from](const Edge& e) { return e.to == from; }),
            edges.end());
    }
}

bool Graph::hasVertex(int id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return vertices_.find(id) != vertices_.end();
}

bool Graph::hasEdge(int from, int to) const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (adjacency_list_.find(from) == adjacency_list_.end()) {
        return false;
    }
    const auto& edges = adjacency_list_.at(from);
    return std::any_of(edges.begin(), edges.end(),
        [to](const Edge& e) { return e.to == to; });
}

std::vector<int> Graph::getNeighbors(int id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<int> neighbors;
    if (adjacency_list_.find(id) != adjacency_list_.end()) {
        for (const auto& edge : adjacency_list_.at(id)) {
            neighbors.push_back(edge.to);
        }
    }
    return neighbors;
}

std::vector<Edge> Graph::getEdges() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Edge> edges;
    std::unordered_set<std::string> seen;
    for (const auto& [from, edge_list] : adjacency_list_) {
        for (const auto& edge : edge_list) {
            std::string key = directed_ 
                ? std::to_string(from) + "," + std::to_string(edge.to)
                : (from < edge.to 
                    ? std::to_string(from) + "," + std::to_string(edge.to)
                    : std::to_string(edge.to) + "," + std::to_string(from));
            if (seen.find(key) == seen.end()) {
                edges.push_back(edge);
                seen.insert(key);
            }
        }
    }
    return edges;
}

std::vector<int> Graph::getVertices() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<int> vertices;
    for (const auto& [id, _] : vertices_) {
        vertices.push_back(id);
    }
    return vertices;
}

int Graph::getVertexCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return static_cast<int>(vertices_.size());
}

int Graph::getEdgeCount() const {
    return static_cast<int>(getEdges().size());
}

bool Graph::isDirected() const {
    return directed_;
}

Vertex* Graph::getVertex(int id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = vertices_.find(id);
    return (it != vertices_.end()) ? it->second.get() : nullptr;
}

const Vertex* Graph::getVertex(int id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = vertices_.find(id);
    return (it != vertices_.end()) ? it->second.get() : nullptr;
}

Edge Graph::getEdge(int from, int to) const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (adjacency_list_.find(from) != adjacency_list_.end()) {
        for (const auto& edge : adjacency_list_.at(from)) {
            if (edge.to == to) {
                return edge;
            }
        }
    }
    return Edge(0, 0, 0.0);
}

void Graph::setVertexPosition(int id, double x, double y) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (vertices_.find(id) != vertices_.end()) {
        vertices_[id]->x = x;
        vertices_[id]->y = y;
    }
}

int Graph::getDegree(int id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (adjacency_list_.find(id) == adjacency_list_.end()) {
        return 0;
    }
    return static_cast<int>(adjacency_list_.at(id).size());
}

double Graph::getDensity() const {
    std::lock_guard<std::mutex> lock(mutex_);
    int n = static_cast<int>(vertices_.size());
    if (n < 2) return 0.0;
    int m = getEdgeCount();
    int max_edges = directed_ ? n * (n - 1) : n * (n - 1) / 2;
    return max_edges > 0 ? static_cast<double>(m) / max_edges : 0.0;
}

std::vector<std::vector<int>> Graph::getConnectedComponents() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::vector<int>> components;
    std::unordered_set<int> visited;
    
    std::function<void(int, std::vector<int>&)> dfs = [&](int v, std::vector<int>& component) {
        visited.insert(v);
        component.push_back(v);
        if (adjacency_list_.find(v) != adjacency_list_.end()) {
            for (const auto& edge : adjacency_list_.at(v)) {
                if (visited.find(edge.to) == visited.end()) {
                    dfs(edge.to, component);
                }
            }
        }
    };
    
    for (const auto& [id, _] : vertices_) {
        if (visited.find(id) == visited.end()) {
            std::vector<int> component;
            dfs(id, component);
            components.push_back(component);
        }
    }
    
    return components;
}

} // namespace graph

