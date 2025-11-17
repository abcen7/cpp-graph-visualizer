#pragma once

#include "core/graph.hpp"
#include <vector>
#include <queue>
#include <functional>
#include <atomic>

namespace graph {

struct AlgorithmState {
    std::atomic<bool> isRunning{false};
    std::atomic<bool> isPaused{false};
    std::atomic<int> currentVertex{-1};
    std::vector<int> visited;
    std::vector<int> path;
    std::mutex stateMutex;
    
    void reset() {
        isRunning = false;
        isPaused = false;
        currentVertex = -1;
        visited.clear();
        path.clear();
    }
};

class Algorithms {
public:
    // BFS обход в ширину
    static std::vector<int> BFS(Graph& g, int start, AlgorithmState& state);
    
    // DFS обход в глубину
    static std::vector<int> DFS(Graph& g, int start, AlgorithmState& state);
    
    // Dijkstra поиск кратчайшего пути
    static std::vector<int> Dijkstra(Graph& g, int start, int end, AlgorithmState& state);
    
    // Вспомогательные функции
    static void waitIfPaused(AlgorithmState& state);
    static void updateState(AlgorithmState& state, int vertex, bool visited = true);
};

} // namespace graph

