#pragma once

#include "core/graph.hpp"
#include "core/algorithms.hpp"
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <future>
#include <type_traits>
#include <stdexcept>

namespace graph {

class ThreadPool {
public:
    ThreadPool(size_t numThreads = std::thread::hardware_concurrency());
    ~ThreadPool();
    
    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type>;
    
    void shutdown();
    
private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queueMutex_;
    std::condition_variable condition_;
    std::atomic<bool> stop_{false};
};

template<typename F, typename... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type> {
    using return_type = typename std::invoke_result<F, Args...>::type;
    
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );
    
    std::future<return_type> result = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queueMutex_);
        if (stop_) {
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }
        tasks_.emplace([task](){ (*task)(); });
    }
    condition_.notify_one();
    return result;
}

class ParallelAlgorithms {
public:
    // Параллельный BFS с разделением уровней
    static std::vector<int> parallelBFS(Graph& g, int start, AlgorithmState& state, size_t numThreads = 4);
    
    // Параллельный DFS с разделением ветвей
    static std::vector<int> parallelDFS(Graph& g, int start, AlgorithmState& state, size_t numThreads = 4);
    
    // Параллельное вычисление степеней вершин
    static std::unordered_map<int, int> parallelComputeDegrees(Graph& g, size_t numThreads = 4);
    
    // Параллельное вычисление компонент связности
    static std::vector<std::vector<int>> parallelConnectedComponents(Graph& g, size_t numThreads = 4);
};

} // namespace graph

