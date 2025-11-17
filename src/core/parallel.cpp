#include "core/parallel.hpp"
#include <algorithm>
#include <unordered_set>
#include <unordered_map>

namespace graph {

ThreadPool::ThreadPool(size_t numThreads) {
    for (size_t i = 0; i < numThreads; ++i) {
        workers_.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(queueMutex_);
                    condition_.wait(lock, [this] { return stop_ || !tasks_.empty(); });
                    if (stop_ && tasks_.empty()) {
                        return;
                    }
                    task = std::move(tasks_.front());
                    tasks_.pop();
                }
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    shutdown();
}

void ThreadPool::shutdown() {
    {
        std::unique_lock<std::mutex> lock(queueMutex_);
        stop_ = true;
    }
    condition_.notify_all();
    for (std::thread& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

std::vector<int> ParallelAlgorithms::parallelBFS(Graph& g, int start, AlgorithmState& state, size_t numThreads) {
    state.reset();
    state.isRunning = true;
    
    std::vector<int> result;
    std::queue<int> currentLevel;
    std::unordered_set<int> visited;
    std::mutex resultMutex;
    
    if (!g.hasVertex(start)) {
        state.isRunning = false;
        return result;
    }
    
    currentLevel.push(start);
    visited.insert(start);
    Algorithms::updateState(state, start);
    
    ThreadPool pool(numThreads);
    
    while (!currentLevel.empty() && state.isRunning) {
        Algorithms::waitIfPaused(state);
        
        std::queue<int> nextLevel;
        std::mutex nextLevelMutex;
        std::vector<std::future<void>> futures;
        
        // Обработать текущий уровень параллельно
        while (!currentLevel.empty()) {
            int vertex = currentLevel.front();
            currentLevel.pop();
            
            result.push_back(vertex);
            Algorithms::updateState(state, vertex);
            
            auto neighbors = g.getNeighbors(vertex);
            auto future = pool.enqueue([&state, neighbors, &visited, &nextLevel, &nextLevelMutex]() {
                for (int neighbor : neighbors) {
                    bool shouldAdd = false;
                    {
                        std::lock_guard<std::mutex> lock(nextLevelMutex);
                        if (visited.find(neighbor) == visited.end()) {
                            visited.insert(neighbor);
                            shouldAdd = true;
                        }
                    }
                    if (shouldAdd) {
                        std::lock_guard<std::mutex> lock(nextLevelMutex);
                        nextLevel.push(neighbor);
                        Algorithms::updateState(state, neighbor);
                    }
                }
            });
            futures.push_back(std::move(future));
        }
        
        // Дождаться завершения всех задач
        for (auto& future : futures) {
            future.wait();
        }
        
        currentLevel = std::move(nextLevel);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    state.isRunning = false;
    return result;
}

std::vector<int> ParallelAlgorithms::parallelDFS(Graph& g, int start, AlgorithmState& state, size_t /* numThreads */) {
    // Для DFS параллелизация сложнее, используем простую версию
    return Algorithms::DFS(g, start, state);
}

std::unordered_map<int, int> ParallelAlgorithms::parallelComputeDegrees(Graph& g, size_t numThreads) {
    std::unordered_map<int, int> degrees;
    std::mutex degreesMutex;
    
    auto vertices = g.getVertices();
    ThreadPool pool(numThreads);
    std::vector<std::future<void>> futures;
    
    // Разделить вершины между потоками
    size_t chunkSize = (vertices.size() + numThreads - 1) / numThreads;
    for (size_t i = 0; i < vertices.size(); i += chunkSize) {
        size_t end = std::min(i + chunkSize, vertices.size());
        std::vector<int> chunk(vertices.begin() + i, vertices.begin() + end);
        
        auto future = pool.enqueue([&, chunk]() {
            for (int v : chunk) {
                int degree = g.getDegree(v);
                std::lock_guard<std::mutex> lock(degreesMutex);
                degrees[v] = degree;
            }
        });
        futures.push_back(std::move(future));
    }
    
    for (auto& future : futures) {
        future.wait();
    }
    
    return degrees;
}

std::vector<std::vector<int>> ParallelAlgorithms::parallelConnectedComponents(Graph& g, size_t /* numThreads */) {
    // Упрощенная версия, полная параллелизация сложна для компонент связности
    return g.getConnectedComponents();
}

} // namespace graph

