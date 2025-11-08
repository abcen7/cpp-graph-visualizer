#include "core/algorithms.hpp"
#include <unordered_set>
#include <unordered_map>
#include <limits>
#include <thread>
#include <chrono>

namespace graph {

void Algorithms::waitIfPaused(AlgorithmState& state) {
    while (state.isPaused && state.isRunning) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void Algorithms::updateState(AlgorithmState& state, int vertex, bool visited) {
    std::lock_guard<std::mutex> lock(state.stateMutex);
    state.currentVertex = vertex;
    if (visited && std::find(state.visited.begin(), state.visited.end(), vertex) == state.visited.end()) {
        state.visited.push_back(vertex);
    }
}

std::vector<int> Algorithms::BFS(Graph& g, int start, AlgorithmState& state) {
    state.reset();
    state.isRunning = true;
    
    std::vector<int> result;
    std::queue<int> queue;
    std::unordered_set<int> visited;
    
    if (!g.hasVertex(start)) {
        state.isRunning = false;
        return result;
    }
    
    queue.push(start);
    visited.insert(start);
    updateState(state, start);
    
    while (!queue.empty() && state.isRunning) {
        waitIfPaused(state);
        
        int current = queue.front();
        queue.pop();
        result.push_back(current);
        
        updateState(state, current);
        
        auto neighbors = g.getNeighbors(current);
        for (int neighbor : neighbors) {
            if (visited.find(neighbor) == visited.end()) {
                visited.insert(neighbor);
                queue.push(neighbor);
                updateState(state, neighbor);
            }
        }
        
        // Небольшая задержка для визуализации
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    state.isRunning = false;
    return result;
}

std::vector<int> Algorithms::DFS(Graph& g, int start, AlgorithmState& state) {
    state.reset();
    state.isRunning = true;
    
    std::vector<int> result;
    std::unordered_set<int> visited;
    
    if (!g.hasVertex(start)) {
        state.isRunning = false;
        return result;
    }
    
    std::function<void(int)> dfs_recursive = [&](int v) {
        if (!state.isRunning || visited.find(v) != visited.end()) {
            return;
        }
        
        waitIfPaused(state);
        
        visited.insert(v);
        result.push_back(v);
        updateState(state, v);
        
        auto neighbors = g.getNeighbors(v);
        for (int neighbor : neighbors) {
            if (visited.find(neighbor) == visited.end()) {
                dfs_recursive(neighbor);
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    };
    
    dfs_recursive(start);
    
    state.isRunning = false;
    return result;
}

std::vector<int> Algorithms::Dijkstra(Graph& g, int start, int end, AlgorithmState& state) {
    state.reset();
    state.isRunning = true;
    
    std::vector<int> path;
    
    if (!g.hasVertex(start) || !g.hasVertex(end)) {
        state.isRunning = false;
        return path;
    }
    
    std::unordered_map<int, double> distances;
    std::unordered_map<int, int> previous;
    std::unordered_set<int> unvisited;
    
    auto vertices = g.getVertices();
    for (int v : vertices) {
        distances[v] = std::numeric_limits<double>::infinity();
        previous[v] = -1;
        unvisited.insert(v);
    }
    distances[start] = 0.0;
    
    while (!unvisited.empty() && state.isRunning) {
        waitIfPaused(state);
        
        // Найти вершину с минимальным расстоянием
        int current = -1;
        double minDist = std::numeric_limits<double>::infinity();
        for (int v : unvisited) {
            if (distances[v] < minDist) {
                minDist = distances[v];
                current = v;
            }
        }
        
        if (current == -1 || minDist == std::numeric_limits<double>::infinity()) {
            break;
        }
        
        unvisited.erase(current);
        updateState(state, current);
        
        if (current == end) {
            // Восстановить путь
            int node = end;
            while (node != -1) {
                path.insert(path.begin(), node);
                node = previous[node];
            }
            break;
        }
        
        auto neighbors = g.getNeighbors(current);
        for (int neighbor : neighbors) {
            if (unvisited.find(neighbor) != unvisited.end()) {
                auto edge = g.getEdge(current, neighbor);
                double alt = distances[current] + edge.weight;
                if (alt < distances[neighbor]) {
                    distances[neighbor] = alt;
                    previous[neighbor] = current;
                }
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    state.path = path;
    state.isRunning = false;
    return path;
}

} // namespace graph

